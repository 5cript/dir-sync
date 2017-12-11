#pragma once

#include "server_fwd.hpp"
#include "cloner.hpp"
#include "progress_report.hpp"

#include <chrono>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>

namespace FileSpreader
{
    class Controller
    {
        friend Server; // not needed as of yet.

    public:
        Controller(int scanMax);
        ~Controller();

        /**
         *  Sets the sleep time, when nothing is TBD.
         */
        void setUpdateInterval(std::chrono::milliseconds const& sleepTime);

        /**
         *  Add a new spreader task.
         */
        void addTask(std::string const& source, std::vector <std::string> const& destinations, ClonerOptions const& options = {});

        /**
         *  Removes a spreader task and uses the stop strategy to stop it.
         */
        void removeTask(std::string const& source);

        /**
         *  Creates a list of all currently registered tasks and saves them to a file.
         *  This file can later be used to reload the tasks back.
         *
         *  @param The file to save to. Will be created or overwritten.
         */
        bool saveTasksToFile(std::string const& fileName);

        /**
         *  Loads a previously saved set of tasks from the file.
         *
         *  @see saveTasksToFile.
         *
         *  @param fileName The file to load.
         */
        bool loadTasksFromFile(std::string const& fileName);

        /**
         *  Returns the error message that was lastly thrown in the pulser thread.
         */
        std::string getLastError();

        /**
         *  Collects all kinds of progress information and returns it.
         */
        ProgressReport compileProgressReport(bool verbose, bool calculateSizeTotals = false) const;

        void start();
        void pause();
        void stop(); // = pause: there is no semantic difference.

        bool isRunning() const;
        bool isRefreshing() const;

    private:
        void refreshAllCloners(bool force = false);
        bool pulseAllCloners(int scanMax);

    private:
        std::chrono::milliseconds interval_;
        std::map <std::string /* source */, Cloner> cloners_;
        std::thread pulser_;
        std::atomic_bool running_;
        std::string lastError_;
        int scanMax_;
    };
}
