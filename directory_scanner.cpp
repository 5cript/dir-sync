#include "directory_scanner.hpp"
#include "archive_bit.hpp"
#include "log.hpp"

#include <algorithm>
#include <iterator>
#include <boost/filesystem.hpp>

namespace FileSpreader
{
//#####################################################################################################################
    namespace fs = boost::filesystem;
//#####################################################################################################################
    DirectoryScanner::DirectoryScanner(std::string directory, ClonerOptions options, bool filtered)
        : sourceDirectory_{std::move(directory)}
        , scannerIterator_{}
        , options_{std::move(options)}
        , filtered_{filtered}
        , differenceProgress_{-1}
        , list_{}
        , sievedList_{}
    {
        if (fs::exists(sourceDirectory_) && !fs::is_directory(sourceDirectory_))
        {
            Log(LogSeverity::Severe, "An added task is invalid.", LOG_CODE_PLACE);
            throw std::runtime_error("scanner must operate on a directory");
        }

        reset();
    }
//---------------------------------------------------------------------------------------------------------------------
    DirectoryScanner::~DirectoryScanner() = default;
//---------------------------------------------------------------------------------------------------------------------
    std::string DirectoryScanner::getDirectory() const
    {
        return sourceDirectory_;
    }
//---------------------------------------------------------------------------------------------------------------------
    void DirectoryScanner::setOptions(ClonerOptions const& options)
    {
        options_ = options;
    }
//---------------------------------------------------------------------------------------------------------------------
    void DirectoryScanner::reset()
    {
        list_.clear();
        filesScanned_ = 0;
        differenceProgress_ = -1;
        scannerIterator_ = fs::recursive_directory_iterator{sourceDirectory_};
    }
//---------------------------------------------------------------------------------------------------------------------
    bool DirectoryScanner::finished() const
    {
        return (scannerIterator_ == fs::recursive_directory_iterator{});
    }
//---------------------------------------------------------------------------------------------------------------------
    uint64_t DirectoryScanner::getFileCount() const
    {
        return filesScanned_;
    }
//---------------------------------------------------------------------------------------------------------------------
    int DirectoryScanner::scan(int amount)
    {
        using namespace std::string_literals;

        if (finished())
            return 0;

        if (!fs::exists(sourceDirectory_))
            fs::create_directory(sourceDirectory_);
        if (!fs::exists(sourceDirectory_))
        {
            Log(LogSeverity::Severe, "Cannot create a directory for the task", LOG_CODE_PLACE);
            throw std::runtime_error("Cannot find nor create the assigned directory");
        }

        auto basePathLen = sourceDirectory_.length();
        auto& opts = options_.getDestinationOptions(sourceDirectory_);
        auto suffixFilter = WildcardFilter{"*"s + options_.getTempSuffix()};
        auto end = fs::recursive_directory_iterator{};
        auto i = 0;

        for (; scannerIterator_ != end && i != amount; ++i, ++scannerIterator_)
        {
            if (fs::is_regular_file(scannerIterator_->path()))
            {
                // relative path from source_
                auto pathString = scannerIterator_->path().string();
                pathString.erase(0, basePathLen);

                // check for filters, and if not filtered, add it to the set.
                if (!filtered_ || !opts.filtered(pathString, &suffixFilter))
                    list_.insert(pathString);

                ++filesScanned_;
            }
        }
        return i;
    }
//---------------------------------------------------------------------------------------------------------------------
    DirectoryScanner::PathContainerType* DirectoryScanner::getList()
    {
        sievedList_ = list_;
        return &sievedList_;
    }
//---------------------------------------------------------------------------------------------------------------------
    bool DirectoryScanner::findDifference(
        SymmetricDifferenceExtractor <PathContainerType::iterator>& differenceFinder,
        DirectoryScanner const& other,
        int amount
    ) const
    {
        using namespace std::string_literals;

        if (differenceProgress_ == -1)
        {
            bool done = differenceFinder.workLeftOnly(amount);

            if (!done)
                return true;
            else
                differenceProgress_ = 0;
        }

        if (options_.isUsingArchiveBit())
        {
            int scaledAmount = amount/40;
            if (scaledAmount < 1)
                scaledAmount = 1;

            auto& uni = *differenceFinder.getUnion();
            auto end = std::begin(uni) + differenceProgress_ + scaledAmount;
            bool done = false;
            if (end >= std::end(uni))
            {
                end = std::end(uni);
                done = true;
            }

            auto cutOffBegin = std::remove_if(
                std::begin(uni) + differenceProgress_,
                end,
                [this](auto const& elem)
                {
                    return getArchiveBit(sourceDirectory_ + elem) == ArchiveBitState::Clean;
                }
            );

            int notDeletedAmount = cutOffBegin - std::begin(uni);
            uni.erase(cutOffBegin, end);
            differenceProgress_ = notDeletedAmount;

            if (done)
                differenceProgress_ = -1;

            return !done;
        }
        return false;
    }
//#####################################################################################################################
}
