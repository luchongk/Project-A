#ifndef PLATFORM_WIN32_H
#define PLATFORM_WIN32_H

#include <windows.h>
#include <windowsx.h>

#include "platform.h"

struct GameCode {
    HMODULE dll;
    FILETIME dllLastWriteTime;
    GameAPI api;
};

struct Win32State {  
    PlayerInput input;

    bool borderlessFullscreen;
    WINDOWPLACEMENT windowPlacement{ sizeof(WINDOWPLACEMENT) };
    LARGE_INTEGER startTimestamp;
    LARGE_INTEGER timerFrequency;
    bool quit;
};

#endif