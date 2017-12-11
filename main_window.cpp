#include "main_window.hpp"

//---------------------------------------------------------------------------------------------------------------------
HWND window;
//---------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CLOSE:
            DestroyWindow(window);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}
//---------------------------------------------------------------------------------------------------------------------
bool CreateHiddenWindow(MSG& messages, HINSTANCE& instance)
{
    WNDCLASSEX wincl;

    wincl.hInstance = instance;
    wincl.lpszClassName = windowClassName;
    wincl.lpfnWndProc = WindowProcedure;      // WndProc
    wincl.style = CS_DBLCLKS;                 // catch double clicks
    wincl.cbSize = sizeof (WNDCLASSEX);

    // default icon and mouse pointer
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 // no menu
    wincl.cbClsExtra = 0;                      // no extra bytes after the window class
    wincl.cbWndExtra = 0;                      // structure or the window instance
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND; // Default background color

    // Register the window class, and if it fails quit the program.
    if (!RegisterClassEx (&wincl))
        return false;

    // The class is registered, let's create the program
    window = CreateWindowEx (
       0,                   // Extended possibilites for variation
       windowClassName,     // class name
       windowCaption,       // title text
       WS_OVERLAPPEDWINDOW, // default window
       CW_USEDEFAULT,       // windows decides the position
       CW_USEDEFAULT,       // where the window ends up on the screen
       544,                 // the programs width
       375,                 // and height in pixels
       HWND_DESKTOP,        // the window is a child-window to desktop
       NULL,                // no menu
       instance,            // program Instance handler
       NULL                 // no Window Creation data
    );

    ShowWindow (window, SW_HIDE);
    return true;
}
//---------------------------------------------------------------------------------------------------------------------
