#include "cloner.hpp"
#include "filter.hpp"
#include "log.hpp"

#include <boost/filesystem.hpp>

#include <thread>

namespace FileSpreader
{
    namespace fs = boost::filesystem;
    using namespace std::string_literals;
//#####################################################################################################################
    Cloner::Cloner(std::string source,
                   std::vector <std::string> const& destinations,
                   ClonerOptions const& options)
        : source_{std::move(source), options, true}
        , destinations_{[&]() {
            std::vector <DirectoryScanner> scanners;
            for (auto const& i : destinations)
                scanners.emplace_back(i, options, false);
            return scanners;
        }()}
        , options_{options}
        , progressReportStop_{}
        , runningCopyProcesses_{}
        , differences_{}
        , differenceBuilt_{false}
        , lastWorkTime_{std::chrono::system_clock::now()}
    {
    }
//---------------------------------------------------------------------------------------------------------------------
    void Cloner::createNewCopier(std::string const& destination)
    {
        auto* diffPtr = differences_.find(destination)->second.getLeftDifference();

        if (diffPtr->empty() && !options_.isUsingArchiveBit())
            return;

        else if (diffPtr->empty())
        {
            diffPtr = differences_.find(destination)->second.getUnion();
            if (diffPtr->empty())
                return;
        }

        auto& diff = *diffPtr;

        auto sourceFile = fs::path(source_.getDirectory()) / diff.back();
        //auto destinationFile = getDestinationFromSource(sourceFile, destination);
        auto destinationFile = fs::path(destination) / fs::path(diff.back());

        auto dir = destinationFile.parent_path();
        if (!fs::exists(dir))
            if (!recursiveCreateDirectory(dir.string()))
                return;

        //if (Copier::testFileAccess(sourceFile, destinationFile, options_.getTempSuffix()))
        {
            runningCopyProcesses_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(destination),
                std::forward_as_tuple(
                    sourceFile.string(),
                    destinationFile.string(),
                    options_.isUsingArchiveBit(),
                    options_.getTempSuffix()
                )
            );

            Log(LogSeverity::Debug, "Started: "s + destinationFile.make_preferred().string() + ".");
        }

        // remove file from todo-list
        diff.pop_back();
    }
//---------------------------------------------------------------------------------------------------------------------
    ClonerOptions Cloner::getOptions() const
    {
        return options_;
    }
//---------------------------------------------------------------------------------------------------------------------
    std::string Cloner::getSource() const
    {
        return source_.getDirectory();
    }
//---------------------------------------------------------------------------------------------------------------------
    std::vector <std::string> Cloner::getDestinations() const
    {
        std::vector <std::string> result;
        for (auto const& i : destinations_)
            result.push_back(i.getDirectory());
        return result;
    }
//---------------------------------------------------------------------------------------------------------------------
    std::string Cloner::getDestinationFromSource(std::string const& sourceFile, std::string const& destinationRoot) const
    {
        auto subPath = sourceFile.substr(source_.getDirectory().length(), sourceFile.length() - source_.getDirectory().length());
        return (fs::path(destinationRoot) / fs::path(subPath)).string();
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Cloner::recursiveCreateDirectory(std::string const& directory)
    {
        return fs::create_directories(directory);
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Cloner::needsRefresh(std::chrono::milliseconds const& interval) const
    {
        std::unique_lock <decltype(progressReportStop_)> reportLock(progressReportStop_, std::defer_lock);
        if (!reportLock.try_lock())
            return false;

        for (auto const& i : differences_)
            if (!i.second.isEmptyLeft())
                return false;

        return std::chrono::system_clock::now() - lastWorkTime_ > std::chrono::milliseconds(interval);
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Cloner::hasEmptyRemainingFilesList() const
    {
        /** do not lock here, lock one layer up **/

        bool somethingToDo = false;
        for (auto const& i : differences_)
        {
            if (!i.second.isEmptyLeft())
            {
                somethingToDo = true;
                break;
            }
        }
        return somethingToDo;
    }
//---------------------------------------------------------------------------------------------------------------------
    void Cloner::refresh()
    {
        differenceBuilt_ = false;

        source_.reset();
        differences_.clear();

        for (auto& i : destinations_)
            i.reset();
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Cloner::scanDone() const
    {
        if (!source_.finished())
            return false;

        for (auto const& i : destinations_)
            if (!i.finished())
                return false;

        return true;
    }
//---------------------------------------------------------------------------------------------------------------------
    void Cloner::scan(int amount)
    {
        if (scanDone())
            return;

        source_.scan(amount);

        uint64_t count = source_.getFileCount();
        for (auto& i : destinations_)
        {
            i.scan(amount);
            count += i.getFileCount();
        }

        if (count > 0)
            Log(LogSeverity::Info, std::to_string(count) + " files scanned.");
    }
//---------------------------------------------------------------------------------------------------------------------
    void Cloner::findDifference(int amount)
    {
        if (!scanDone())
            return;

        differenceBuilt_ = true;
        for (auto& i : destinations_)
        {
            auto diff = differences_.find(i.getDirectory());
            if (diff == std::end(differences_))
            {
                differences_.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(i.getDirectory()),
                    std::forward_as_tuple(
                        source_.getList(),
                        i.getList()
                    )
                );
                diff = differences_.find(i.getDirectory());
            }
            differenceBuilt_ &= !source_.findDifference(diff->second, i, amount);
        }

        if (differenceBuilt_)
            Log(LogSeverity::Info, "File difference determined.");
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Cloner::differenceFound() const
    {
        return differenceBuilt_;
    }
//---------------------------------------------------------------------------------------------------------------------
    boost::optional <SourceGroupProgress> Cloner::compileProgressReport(bool verbose) const
    {
        std::unique_lock <decltype(progressReportStop_)> reportLock(progressReportStop_, std::defer_lock);
        if (!reportLock.try_lock_for(std::chrono::seconds(1)))
            return boost::none;

        SourceGroupProgress result;
        result.sourceFileCount = source_.getFileCount();

        for (auto const& desti : destinations_)
        {
            DestinationProgress desProg;
            desProg.destination = desti.getDirectory();
            desProg.remainingFileCount = differences_.find(desti.getDirectory())->second.leftSize();
            desProg.scanFileCount = desti.getFileCount();

            auto runningCpy = runningCopyProcesses_.find(desti.getDirectory());
            if (runningCpy != std::end(runningCopyProcesses_))
            {
                desProg.currentFile = runningCpy->second.getDestinationFile();
                if (runningCpy->second.getProgressMax() != 0)
                    desProg.currentFileProgress = (100. * runningCpy->second.getProgress()) / runningCpy->second.getProgressMax();
                else
                    desProg.currentFileProgress = 100.;
            }
            else
            {
                desProg.currentFile = "";
                desProg.currentFileProgress = 0.;
            }

            auto remFiles = differences_.find(desti.getDirectory());
            if (remFiles != std::end(differences_))
            {
                if (verbose)
                    desProg.remainingFiles = remFiles->second.getLeftDifference();

                desProg.remainingFileCount = remFiles->second.leftSize();
            }
            else
                desProg.remainingFiles = {};

            result.destinations.push_back(desProg);
        }

        return boost::optional <SourceGroupProgress>{result};
    }
//---------------------------------------------------------------------------------------------------------------------
    void Cloner::reset()
    {
        std::lock_guard <decltype(progressReportStop_)> reportLock(progressReportStop_);

        runningCopyProcesses_.clear();
        differences_.clear();
    }
//---------------------------------------------------------------------------------------------------------------------
    void Cloner::tryAssignTasks()
    {
        if (runningCopyProcesses_.size() == destinations_.size())
            return;

        for (auto const& i : destinations_)
        {
            auto m = runningCopyProcesses_.find(i.getDirectory());
            if (m != std::end(runningCopyProcesses_))
                continue; // there is a running task for this destination

            // assign new work
            createNewCopier(i.getDirectory());
        }
    }
//---------------------------------------------------------------------------------------------------------------------
    void Cloner::clearFinishedTasks()
    {
        std::vector <std::string> eraseList;

        for (auto& i : runningCopyProcesses_)
        {
            if (i.second.isDone())
            {
                eraseList.push_back(i.first);
                Log(LogSeverity::Debug, "Finished: "s + fs::path(i.second.getDestinationFile()).make_preferred().string() + ".");
            }
            else if (!i.second.isGood())
            {
                eraseList.push_back(i.first);
                Log(LogSeverity::Warning, "Removed ill copy process involving: "s + fs::path(i.second.getDestinationFile()).make_preferred().string() + ".");
            }
        }

        for (auto const& i : eraseList)
            runningCopyProcesses_.erase(i);
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Cloner::pulse(int scanMax)
    {
        std::unique_lock <decltype(progressReportStop_)> reportLock(progressReportStop_, std::defer_lock);
        if (!reportLock.try_lock())
            return true;

        // we are scanning now, so scan and return true (work was done)
        if (!scanDone())
        {
            scan(scanMax);
            lastWorkTime_ = std::chrono::system_clock::now();
            return true;
        }

        if (!differenceFound())
        {
            findDifference(100'000);
            lastWorkTime_ = std::chrono::system_clock::now();
            return true;
        }

        // remove finished copy processes.
        clearFinishedTasks();

        // fill "runningCopyProcesses_"
        tryAssignTasks();

        // copy a chunk for every open file
        for (auto& i : runningCopyProcesses_)
            i.second.copyChunk();

        if (!runningCopyProcesses_.empty())
            lastWorkTime_ = std::chrono::system_clock::now();

        return !runningCopyProcesses_.empty();
    }
//#####################################################################################################################
}
