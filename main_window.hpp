#pragma once

#include "windows.hpp"

extern HWND window;

LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

const TCHAR windowClassName[] = _T("IWS_File_Copier");
const TCHAR windowCaption[] = _T("IWS_File_Copier");

bool CreateHiddenWindow(MSG& messages, HINSTANCE& instance);
