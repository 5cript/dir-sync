#include "controller.hpp"
#include "messages/task.hpp"
#include "log.hpp"

#include <boost/filesystem.hpp>
#include <chrono>
#include <fstream>
#include <algorithm>

#include "windows.hpp"

namespace FileSpreader
{
    namespace fs = boost::filesystem;
    using namespace std::string_literals;
//#####################################################################################################################
    std::ifstream::pos_type getFileSize(std::string const& file)
    {
        std::ifstream reader (file, std::ios_base::binary | std::ios_base::ate);
        if (reader.good())
            return reader.tellg();
        else
            return 0;
    }
//#####################################################################################################################
    Controller::Controller(int scanMax)
        : interval_(5000)
        , cloners_()
        , pulser_()
        , running_(false)
        , scanMax_{scanMax}
    {
        Log("Controller created.");
    }
//---------------------------------------------------------------------------------------------------------------------
    void Controller::setUpdateInterval(std::chrono::milliseconds const& sleepTime)
    {
        interval_ = sleepTime;
        Log("Interval set to "s + std::to_string(sleepTime.count()) + "ms.");
    }
//---------------------------------------------------------------------------------------------------------------------
    void Controller::addTask(std::string const& source, std::vector <std::string> const& destinations, ClonerOptions const& options)
    {
        // overwrite existing task.
        auto task = cloners_.find(source);
        if (task != cloners_.end())
            removeTask(source);

        using namespace std::string_literals;

        for (auto const& i : destinations)
        {
            if (!fs::exists(fs::path(i)))
                if (!fs::create_directory(i))
                    throw std::runtime_error(("could not create one of the destination directory: " + i).c_str());
        }

        cloners_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(source),
            std::forward_as_tuple(
                source,
                destinations,
                options
            )
        );

        Log("Task added with source: "s + source + ".");
    }
//---------------------------------------------------------------------------------------------------------------------
    void Controller::removeTask(std::string const& source)
    {
        cloners_.erase(source);
        Log("Task removed with source: "s + source + ".");
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Controller::saveTasksToFile(std::string const& fileName)
    {
        using namespace Messages;

        TaskList list;

        auto filterListConvert = [](std::vector <WildcardFilter> const& filterList) {
            std::vector <std::string> res;
            for (auto const& i : filterList)
                res.push_back(i);
            return res;
        };

        for (auto const& i : cloners_)
        {
            auto options = i.second.getOptions();

            Task task;
            task.source = i.second.getSource();
            task.useArchiveBit = options.isUsingArchiveBit();

            for (auto const& d : i.second.getDestinations())
            {
                Messages::Destination dest;
                dest.directory = d;

                auto whiteList = options.getDestinationOptions(d).getWhiteList();
                if (!whiteList.empty())
                    dest.whiteList = filterListConvert(whiteList);

                auto blackList = options.getDestinationOptions(d).getBlackList();
                if (!blackList.empty())
                    dest.blackList = filterListConvert(blackList);

                if (!options.getDestinationOptions(d).getRegexBlackList())
                    dest.blackListRegex = options.getDestinationOptions(d).getRegexBlackList();

                if (!options.getDestinationOptions(d).getRegexWhiteList())
                    dest.whiteListRegex = options.getDestinationOptions(d).getRegexWhiteList();

                task.destinations.push_back(dest);
            }

            list.push_back(task);
        }

        std::ofstream writer(fileName, std::ios_base::binary);

        if (!writer.good())
            return false;

        writer << "{";
        JSON::try_stringify(writer, "task_collection", list, JSON::ProduceNamedOutput);
        writer << "}";

        Log("Tasks saved to "s + fileName + ".");

        return true;
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Controller::loadTasksFromFile(std::string const& fileName)
    {
        using namespace Messages;

        TaskList list;

        std::ifstream file(fileName, std::ios_base::binary);
        if (!file.good())
            return false;

        auto tree = JSON::parse_json(file);
        JSON::parse(list, "task_collection", tree);

        for (auto const& i : list)
        {
            std::vector <std::string> destinations = i.getDestinations();
            ClonerOptions options = ClonerOptionsFromMessage(i);

            addTask(i.source, destinations, options);
        }

        Log("Tasks loaded from "s + fileName + ".");

        return true;
    }
//---------------------------------------------------------------------------------------------------------------------
    std::string Controller::getLastError()
    {
        auto le = lastError_;
        lastError_.clear();
        return le;
    }
//---------------------------------------------------------------------------------------------------------------------
    ProgressReport Controller::compileProgressReport(bool verbose, bool calculateSizeTotals) const
    {
        ProgressReport result;
        result.totalRemainingBytes = 0;
        result.totalRemainingFiles = 0;

        for (auto const& i : cloners_)
        {
            auto report = i.second.compileProgressReport(verbose);
            if (report)
            {
                result.sources.push_back(report.get());

                for (auto const& i : report.get().destinations)
                {
                    result.totalRemainingFiles += i.remainingFileCount;
                }
            }
        }

        for (auto& task : result.sources)
        {
            for (auto& sink : task.destinations)
            {
                std::replace(sink.destination.begin(), sink.destination.end(), '/', '\\');
                result.totalRemainingFiles += sink.remainingFiles.size();

                if (verbose)
                    for (auto& remFile : sink.remainingFiles)
                    {
                        if (calculateSizeTotals)
                            result.totalRemainingBytes += getFileSize(remFile);
                        std::replace(remFile.begin(), remFile.end(), '/', '\\');
                    }

                std::replace(sink.currentFile.begin(), sink.currentFile.end(), '/', '\\');
            }
        }

        Log(LogSeverity::Debug, "Progress report assembled.");

        return result;
    }
//---------------------------------------------------------------------------------------------------------------------
    void Controller::refreshAllCloners(bool force)
    {
        for (auto& i : cloners_)
            if (force || i.second.needsRefresh(interval_))
                i.second.refresh();
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Controller::isRunning() const
    {
        return running_.load();
    }
//---------------------------------------------------------------------------------------------------------------------
    void Controller::start()
    {
        pause();

        pulser_ = std::thread
        {
            [this, scanMax=scanMax_]()
            {
                running_.store(true);
                Log("Controller start.");

                try
                {
                    for (;running_.load();)
                    {
                        std::this_thread::yield();

                        bool didSomeWork = pulseAllCloners(scanMax);

                        std::this_thread::yield();

                        if (!didSomeWork)
                            std::this_thread::sleep_for(std::chrono::milliseconds(50));

                        // refreshes the clones, if the need to be refreshed.
                        refreshAllCloners();
                    }
                }
                catch (std::exception const& exc)
                {
                    running_.store(false);
                    lastError_ = exc.what();
                    Log(LogSeverity::Severe, "Exception in pulser caught. Stopping Controller.", LOG_CODE_PLACE);
                    Log(LogSeverity::Severe, exc.what(), LOG_CODE_PLACE);
                }
            }
        };
    }
//---------------------------------------------------------------------------------------------------------------------
    void Controller::pause()
    {
        running_.store(false);
        if (pulser_.joinable())
        {
            pulser_.join();
            Log("Controller paused.");
        }
    }
//---------------------------------------------------------------------------------------------------------------------
    void Controller::stop()
    {
        pause();
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Controller::pulseAllCloners(int scanMax)
    {
        bool workDone = false;
        for (auto& i : cloners_)
            workDone |= i.second.pulse(scanMax);

        return workDone;
    }
//---------------------------------------------------------------------------------------------------------------------
    Controller::~Controller()
    {
        stop();
    }
//#####################################################################################################################
}
