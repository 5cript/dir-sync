#include "copier.hpp"
#include "archive_bit.hpp"
#include "log.hpp"

#include <boost/filesystem.hpp>

namespace FileSpreader
{
    namespace fs = boost::filesystem;

//#####################################################################################################################
    std::string getTempFileName(std::string const& fileName, std::string const& postfix)
    {
        auto base = fs::path(fileName).parent_path();
        return (base / fs::path(fs::path(fileName).filename().string() + postfix)).string();
    }
//#####################################################################################################################
    Copier::Copier(std::string const& source, std::string const& destination, bool useArchiveBit,
                   std::string const& tempPostfix, uint64_t chunkSize)
        : actualFile_(destination)
        , tempFile_(getTempFileName(destination, tempPostfix))
        , sourceFileName_(source)
        , source_(source, std::ios_base::binary)
        , destination_(tempFile_, std::ios_base::binary)
        , buffer_(chunkSize)
        , copiedBytes_(0)
        , totalFileSize_(0)
        , illState_(false)
        , useArchiveBit_(useArchiveBit)
    {
        if (source_.good())
        {
            source_.seekg(0, std::ios_base::end);
            totalFileSize_ = source_.tellg();
            source_.seekg(0);
        }
        else
        {
            illState_ = true;
        }
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Copier::testFileAccess(std::string const& source, std::string const& destination,
                                std::string const& tempPostfix, bool deleteAfterTest)
    {
        bool good;
        {
            std::ifstream readTest(source, std::ios_base::binary);
            std::ofstream writeTest(getTempFileName(destination, tempPostfix), std::ios_base::binary);

            good = readTest.good() && writeTest.good();
        }

        try
        {
            if (deleteAfterTest || !good)
                fs::remove(getTempFileName(destination, tempPostfix));
        }
        catch (std::exception const& exc)
        {
            Log(LogSeverity::Error, exc.what(), LOG_CODE_PLACE);
        }

        return good;
    }
//---------------------------------------------------------------------------------------------------------------------
    Copier::~Copier()
    {
        destination_.close();

        if (isDone())
        {
            try
            {
                fs::rename(tempFile_, actualFile_);

                if (useArchiveBit_)
                {
                    setArchiveBit(actualFile_, ArchiveBitState::Clean); // protects against circular copy setups.
                    setArchiveBit(sourceFileName_, ArchiveBitState::Clean);
                }
            }
            catch(std::exception const& exc)
            {
                Log(LogSeverity::Error, exc.what(), LOG_CODE_PLACE);
                try
                {
                    fs::remove(tempFile_);
                }
                catch(std::exception const& exc)
                {
                    Log(LogSeverity::Severe, exc.what(), LOG_CODE_PLACE);
                }
            }
        }
        else
        {
            try
            {
                fs::remove(tempFile_);
            }
            catch(std::exception const& exc)
            {
                Log(LogSeverity::Error, exc.what(), LOG_CODE_PLACE);
            }
        }
    }
//---------------------------------------------------------------------------------------------------------------------
    std::string Copier::getDestinationFile() const
    {
        return actualFile_;
    }
//---------------------------------------------------------------------------------------------------------------------
    std::string Copier::getSourceFile() const
    {
        return sourceFileName_;
    }
//---------------------------------------------------------------------------------------------------------------------
    std::string Copier::getSourceFileName() const
    {
        return fs::path(sourceFileName_).filename().string();
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Copier::isGood() const
    {
        return !illState_ && source_.good() && destination_.good();
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Copier::isDone() const
    {
        return !illState_ && getProgress() == getProgressMax();
    }
//---------------------------------------------------------------------------------------------------------------------
    uint64_t Copier::getProgress() const
    {
        return copiedBytes_;
    }
//---------------------------------------------------------------------------------------------------------------------
    uint64_t Copier::getProgressMax() const
    {
        return totalFileSize_;
    }
//---------------------------------------------------------------------------------------------------------------------
    void Copier::copyChunk()
    {
        if (source_.good())
        {
            source_.read(buffer_.data(), buffer_.size());
            destination_.write(buffer_.data(), source_.gcount());
            destination_ << std::flush;

            copiedBytes_ += source_.gcount();
        }
    }
//#####################################################################################################################
}
