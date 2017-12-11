#pragma once

#include "cloner_options.hpp"
#include "set_symmetry.hpp"

#include <boost/filesystem.hpp>

#include <vector>
#include <string>
#include <memory>

namespace FileSpreader
{
    class DirectoryScanner
    {
    public:
        using PathContainerType = std::set <std::string>;

    public:
        DirectoryScanner(std::string directory, ClonerOptions options, bool filtered = false);
        ~DirectoryScanner();

        /**
         *  Set options.
         *  Please note that filters will not apply retroactively to the already made list.
         */
        void setOptions(ClonerOptions const& options);

        /**
         *  resets scanner.
         *  Clears already listed files and resets directory iterator.
         */
        void reset();

        /**
         *  Can more files be listed?
         *  @return Returns false if there are no more files to list.
         */
        bool finished() const;

        /**
         *  Scans over the directory for some more files.
         *
         *  @return Returns the amount of files scanned.
         */
        int scan(int amount = 8192);

        /**
         *  Returns the amount of files that were found.
         */
        uint64_t getFileCount() const;

        /**
         *  Returns the directory this scanner is operating on.
         */
        std::string getDirectory() const;

        /**
         *  Create a file difference between this and other, using this as a base.
         *  This difference only works from left to right.
         *  It will take this as the source and "other" as the destination.
         *  This means that the archive bit setting will apply to files in this->list_.
         *
         *  @param other Another scanner.
         *  @param relative Shall the result be returned relative to the source directory?
         *
         *  @return Returns true if more work is to be done.
         *
         */
        bool findDifference(
            SymmetricDifferenceExtractor <PathContainerType::iterator>& differenceFinder,
            DirectoryScanner const& other,
            int amount = 16000
        ) const;

        PathContainerType* getList();

    private:
        std::string sourceDirectory_;
        boost::filesystem::recursive_directory_iterator scannerIterator_;
        ClonerOptions options_;
        bool filtered_;
        mutable int differenceProgress_;
        uint64_t filesScanned_;

        PathContainerType list_;
        PathContainerType sievedList_;
    };
}
