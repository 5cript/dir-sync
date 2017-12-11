#pragma once

#include <string>

namespace FileSpreader
{
    enum class ArchiveBitState
    {
        Clean = 0,
        Dirty = 1
    };

    // 0 = backed up
    // 1 = THERE WERE CHANGES!
    ArchiveBitState getArchiveBit(std::string const& fileName);
    void setArchiveBit(std::string const& fileName, ArchiveBitState state);
}
