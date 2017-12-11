#include "archive_bit.hpp"

#include <windows.h>

namespace FileSpreader
{
//#####################################################################################################################
    ArchiveBitState getArchiveBit(std::string const& fileName)
    {
        return (GetFileAttributes(fileName.c_str()) & 0x20) ? ArchiveBitState::Dirty : ArchiveBitState::Clean;
    }
//---------------------------------------------------------------------------------------------------------------------
    void setArchiveBit(std::string const& fileName, ArchiveBitState state)
    {
        auto attr = GetFileAttributes(fileName.c_str());
        attr = (attr & ~0x20) | ((int)state * 0x20);
        SetFileAttributes(fileName.c_str(), attr);
    }
//#####################################################################################################################
}
