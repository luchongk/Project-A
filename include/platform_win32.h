#ifndef PLATFORM_WIN32_H
#define PLATFORM_WIN32_H

#include <windows.h>

#include "platform.h"

struct GameCode {
    HMODULE dll;
    FILETIME dllLastWriteTime;
    GameAPI api;
};

#endif