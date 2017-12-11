#include "winutil.hpp"
#include "windows.hpp"

void ShowMessage(std::string const& msg)
{
    MessageBox(nullptr, msg.c_str(), "", MB_OK);
}
