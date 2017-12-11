#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <cstdint>

namespace FileSpreader
{
    class Copier
    {
    public:
        Copier(std::string const& source, std::string const& destination, bool useArchiveBit,
               std::string const& tempPostfix, uint64_t chunkSize = 1 * 1024 * 1024 /* 1 MB */);
        ~Copier();
        Copier(Copier const&) = delete;
        Copier& operator=(Copier const&) = delete;

        bool isGood() const;
        bool isDone() const;

        uint64_t getProgress() const;
        uint64_t getProgressMax() const;

        void copyChunk();

        std::string getDestinationFile() const;
        std::string getSourceFile() const;
        std::string getSourceFileName() const;

        static bool testFileAccess(std::string const& source, std::string const& destination,
                                   std::string const& tempPostfix, bool deleteAfterTest = false);

    private:
        std::string actualFile_;
        std::string tempFile_;
        std::string sourceFileName_;
        std::ifstream source_;
        std::ofstream destination_;
        std::vector <char> buffer_;
        uint64_t copiedBytes_;
        uint64_t totalFileSize_;
        bool illState_;
        bool useArchiveBit_;
    };
}
