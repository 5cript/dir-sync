#pragma once

#include "cloner_options.hpp"
#include "copier.hpp"
#include "progress_report.hpp"
#include "directory_scanner.hpp"
#include "set_symmetry.hpp"

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <set>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <functional>
#include <chrono>

namespace FileSpreader
{
    /**
     *  This class represents a file spread task. 1 Source -> n Destinations
     */
    class Cloner
    {
    public:
        Cloner(std::string source,
               std::vector <std::string> const& destinations,
               ClonerOptions const& options);

        Cloner(Cloner const&) = delete;
        Cloner& operator=(Cloner const&) = delete;

        /**
         *  Copies new chunks for every task.
         *
         *  @return Returns if any work was done.
         */
        bool pulse(int scanMax);

        /**
         *  Stops all copy and resets them (files deleted or renamed, if finished).
         */
        void reset();

        /**
         *  A refresh is needed, if all of the destinations do not have any files copied to and
         *  the refresh wait time has elapsed.
         */
        bool needsRefresh(std::chrono::milliseconds const& interval) const;

        /**
         *  Gets the options back.
         */
        ClonerOptions getOptions() const;

        /**
         *  Gets the source directory.
         */
        std::string getSource() const;

        /**
         *  Gets all destination directories.
         */
        std::vector <std::string> getDestinations() const;

        /**
         *  Get the current progress.
         */
        boost::optional <SourceGroupProgress> compileProgressReport(bool verbose) const;

        /**
         *  Returns whether the directory scanning is complete.
         */
        bool scanDone() const;

        /**
         *  Create a file list of all the differences after scanning.
         */
        void findDifference(int amount);

        /**
         *  Returns true if the difference has been found between source and destination.
         */
        bool differenceFound() const;

        /**
         *  Another directory scan will be performed next pulse, if no work is available
         */
        void refresh();

    private:
        void createNewCopier(std::string const& destination);

        // amount of running processes != destinations.size() ?
        // -> assign new tasks for the destinations.
        void tryAssignTasks();

        void clearFinishedTasks();

        bool hasEmptyRemainingFilesList() const;

        std::string getDestinationFromSource(std::string const& sourceFile, std::string const& destinationRoot) const;
        bool recursiveCreateDirectory(std::string const& directory);

        void scan(int amount);

    private:
        /** The source directory to clone from. **/
        DirectoryScanner source_;

        /** A list of destination directories to copy to. **/
        std::vector <DirectoryScanner> destinations_;

        /** A set of options, including filters etc. **/
        ClonerOptions options_;

        /** A lock that keeps stops copying while a progress report is in the making */
        mutable std::timed_mutex progressReportStop_;

        /** Every destination may have a running copy process **/
        std::map <std::string /* destination dir */, Copier> runningCopyProcesses_;

        /** The difference extractors **/
        std::map <
            std::string /* destination dir */,
            SymmetricDifferenceExtractor <std::set <std::string>::iterator>
        > differences_;

        /** Has the difference been built from the file lists? **/
        bool differenceBuilt_;

        /** Last time work was done **/
        std::chrono::system_clock::time_point lastWorkTime_;
    };
}
