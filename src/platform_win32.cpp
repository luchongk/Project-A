#include <cstdio>

#include "platform_win32.h"
#include "glad/glad.h"

static bool initOpenGL(HDC dc) {
    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 10;

    int pixelFormat = ChoosePixelFormat(dc, &pfd);
    if(!SetPixelFormat(dc, pixelFormat, &pfd)) {
        OutputDebugString(L"Couldn't set pixel format");
        return false;
    }

    HGLRC glContext = wglCreateContext(dc);
    wglMakeCurrent(dc, glContext);

    if(!gladLoadGL()) {
        OutputDebugString(L"Couldn't load glad");
        return false;
    }

    return true;
}

static FILETIME getLastWriteTime(char* filename) {
    FILETIME lastWriteTime{};

    WIN32_FIND_DATAA fileData;
    HANDLE findHandle = FindFirstFileA(filename, &fileData);
    if(findHandle != INVALID_HANDLE_VALUE) {
        FindClose(findHandle);
        lastWriteTime = fileData.ftLastWriteTime;
    }

    return lastWriteTime;
}

static void reloadGameCode(char* gameDLLPath, GameCode* gameCode, GameMemory* gameMemory) {
    if(gameCode->dll) {
        if(!FreeLibrary(gameCode->dll)) {
            printf("Couldn't free game code DLL");
            return;
        }
    }

    gameCode->api.onLoad = onLoadStub;
    gameCode->api.update = updateStub;
    gameCode->api.render = renderStub;

    char* DestDLLPath = "bin\\Debug\\game_tmp.dll";
    
    while(true) {
        if(CopyFileA(gameDLLPath, DestDLLPath, false)) {
            gameCode->dll = LoadLibraryA(DestDLLPath);
            break;
        }
        else if(GetLastError() != ERROR_SHARING_VIOLATION) {
            gameCode->dll = nullptr;
            return;
        }
    }

    gameCode->dllLastWriteTime = getLastWriteTime(gameDLLPath);

    if(!gameCode->dll)
        printf("Failed to load game code DLL\n");
    else {
        gameCode->api.onLoad = (onLoadFunction*)GetProcAddress(gameCode->dll, "onLoad");
        assert(gameCode->api.onLoad);
        
        gameCode->api.update = (updateFunction*)GetProcAddress(gameCode->dll, "update");
        if(!gameCode->api.update) {
            printf("Failed to load render function\n");
            gameCode->api.update = updateStub;
        }
        
        gameCode->api.render = (renderFunction*)GetProcAddress(gameCode->dll, "render");
        if(!gameCode->api.render) {
            printf("Failed to load render function\n");
            gameCode->api.render = renderStub;
        }
    }
}

static bool pollEvents(PlayerInput* input) {
    MSG msg{};

    while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if(msg.message == WM_QUIT) {
            return true;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if(GetKeyState('A') & 0x8000)
        input->horizontal = -1;
    else if(GetKeyState('D') & 0x8000)
        input->horizontal = 1;
    
    if(GetKeyState('S') & 0x8000)
        input->vertical = -1;
    else if(GetKeyState('W') & 0x8000)
        input->vertical = 1;

    return false;
}

inline static float getTimeElapsed(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER freq) {
    assert(start.QuadPart <= end.QuadPart);
    return (end.QuadPart - start.QuadPart) / (float)freq.QuadPart;
}

inline static LARGE_INTEGER getTimestamp() {
    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);
    return current;
}

static size_t get_file_size(const char* path) {
    WIN32_FILE_ATTRIBUTE_DATA file_info;
    assert(GetFileAttributesExA(path, GetFileExInfoStandard, &file_info));
    
    return ((size_t)file_info.nFileSizeHigh << 32) + file_info.nFileSizeLow;
}

static void read_entire_file(const char* path, size_t file_size, void* file_data) {
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    assert(file != INVALID_HANDLE_VALUE);

    DWORD bytesRead;
    assert(ReadFile(file, file_data, (DWORD)file_size, &bytesRead, nullptr));
    assert(file_size == bytesRead);

    CloseHandle(file);

    ((uint8_t*)file_data)[file_size] = '\0';
}

static void set_platform_api(PlatformAPI* platform) {
    platform->get_file_size     = get_file_size;
    platform->read_entire_file  = read_entire_file;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Win32State* platform = (Win32State*)GetProp(hwnd, L"ProjectAWin32State");
    
    switch(uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
        } break;

        case WM_SIZE: {
            glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
        } break;

        case WM_KEYDOWN: {
            uint32_t VKCode = (uint32_t)wParam;
            bool wasDown = lParam & 0x40000000;

            switch(VKCode) {
                case 'P': {
                    if(!wasDown) {
                        platform->input.pause = true;
                    }
                } break;

                case 'R': {
                    if(!wasDown) {
                        platform->input.reset = true;
                    }
                } break;

                case VK_F11: {
                    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
                    if (dwStyle & WS_OVERLAPPEDWINDOW)
                    {
                        MONITORINFOEX mi;
                        mi.cbSize = sizeof(mi);
                        if (GetWindowPlacement(hwnd, &platform->windowPlacement) &&
                            GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
                        {
                            ShowCursor(false);
                            SetWindowLong(hwnd, GWL_STYLE,
                                            dwStyle & ~WS_OVERLAPPEDWINDOW);
                            
                            SetWindowPos(hwnd, platform->borderlessFullscreen ? HWND_TOP : HWND_TOPMOST,
                                        mi.rcMonitor.left, mi.rcMonitor.top,
                                        mi.rcMonitor.right - mi.rcMonitor.left,
                                        mi.rcMonitor.bottom - mi.rcMonitor.top,
                                        SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
                        }
                    } else {
                        ShowCursor(true);
                        SetWindowLong(hwnd, GWL_STYLE,
                                    dwStyle | WS_OVERLAPPEDWINDOW);

                        SetWindowPlacement(hwnd, &platform->windowPlacement);
                        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                                    SWP_NOMOVE | SWP_NOSIZE |
                                    SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
                    }    
                } break;

                case 'F': {
                    platform->borderlessFullscreen = !platform->borderlessFullscreen;
                } break;
            }
        } break;

#if 1
        /* Mouse detection via driver */
        case WM_INPUT: {
            UINT dwSize = sizeof(RAWINPUT);
            RAWINPUT raw;
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &dwSize, sizeof(RAWINPUTHEADER));

            if (raw.header.dwType == RIM_TYPEMOUSE) 
            {
                platform->input.mouseDeltaX += raw.data.mouse.lLastX;
                platform->input.mouseDeltaY += raw.data.mouse.lLastY;
            }

            /*RECT rect;
            GetClientRect(hwnd, &rect);

            POINT center;
            center.x = rect.right / 2;
            center.y = rect.bottom / 2;
            ClientToScreen(hwnd, &center);
            SetCursorPos(center.x, center.y);*/
        } break;
#else
        /* Mouse detection via the screen */
        case WM_MOUSEMOVE: {
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            platform->input.mouseDeltaX += (lParam & 0xFFFF) - rect.right / 2;
            platform->input.mouseDeltaY += ((lParam >> 16) & 0xFFFF) - rect.bottom / 2;

            POINT center;
            center.x = rect.right / 2;
            center.y = rect.bottom / 2;
            ClientToScreen(hwnd, &center);
            SetCursorPos(center.x, center.y);
        } break;
#endif

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"ProjectA";

    WNDCLASS wc{};
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    SetCursor(wc.hCursor);
    wc.style = CS_OWNDC;
    
    if(!RegisterClass(&wc))
        return 1;

    HWND hwnd = CreateWindowW(CLASS_NAME, L"ProjectA", WS_OVERLAPPEDWINDOW,
                                200, 100, 1600, 800,
                                nullptr, nullptr, hInstance, nullptr);

    if(!hwnd)
        return 1;

    Win32State platform{}; 
    SetProp(hwnd, L"ProjectAWin32State", &platform);

    HDC dc = GetDC(hwnd);
    platform.borderlessFullscreen = false;

    if(!initOpenGL(dc))
        return 1;

    ShowWindow(hwnd, nCmdShow);
    /*SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);*/

#if 1
    RAWINPUTDEVICE Rid;
    Rid.usUsagePage = 0x01; 
    Rid.usUsage = 0x02; 
    Rid.dwFlags = 0; //RIDEV_NOLEGACY   // adds HID mouse and also ignores legacy mouse messages
    Rid.hwndTarget = 0;    

    if (!RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE))) {
        /* ERROR */
    }
#endif
    GameMemory memory;
    memory.data_size = megabytes(64);  //TODO: Config
    memory.data = VirtualAlloc(nullptr, memory.data_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    PlatformAPI platform_api;
    set_platform_api(&platform_api);

    char* gameDLLPath = "bin\\Debug\\game.dll";
    GameCode gameCode{};
    reloadGameCode(gameDLLPath, &gameCode, &memory);
    assert(gameCode.dll);
    gameCode.api.onLoad(true, &memory, &platform_api);

    //TODO: Config each of these!
    constexpr float maxFrameTime = 0.25f;
    constexpr float targetFrameTime = 1.0f/60;
    assert(targetFrameTime < maxFrameTime);
    bool capFramerate = false;

    float accum = 0;
    float timeSinceStart = 0;
    float deltaTime = 0;
    float fixedDeltaTime = 1.0f/60;

    timeBeginPeriod(1);
    LARGE_INTEGER frameTime;
    QueryPerformanceFrequency(&platform.timerFrequency);
    QueryPerformanceCounter(&frameTime);
    platform.startTimestamp = frameTime;

    bool quit = false;
    while(!quit) {
        LARGE_INTEGER newFrameTime = getTimestamp();
        deltaTime = getTimeElapsed(frameTime, newFrameTime, platform.timerFrequency);
        timeSinceStart = (float)getTimeElapsed(platform.startTimestamp, newFrameTime, platform.timerFrequency);
        frameTime = newFrameTime;
        
        FILETIME lastWriteTime = getLastWriteTime(gameDLLPath);
        if(CompareFileTime(&gameCode.dllLastWriteTime, &lastWriteTime) != 0) {
            reloadGameCode(gameDLLPath, &gameCode, &memory);
            if(gameCode.api.onLoad)
                gameCode.api.onLoad(false, &memory, &platform_api);
            
            //We need dont want the time we took to reload to count as game_time, so we skip it.
            frameTime = getTimestamp();
        }

        quit = pollEvents(&platform.input);

        if(deltaTime > maxFrameTime) {
            //Framerate too low, we are falling behind in the simulation! D: Preventing spiral of death.
            deltaTime = maxFrameTime;
        }
        
        accum += deltaTime;
        bool updateRan = false;
        while(accum >= fixedDeltaTime) {
            gameCode.api.update(&memory, &platform.input, fixedDeltaTime, timeSinceStart);
            accum -= fixedDeltaTime;
            updateRan = true;
        }
        
        //* RENDERING *//
        //TODO: Stuttering comes from not doing position/rotation interpolation. Do that.
        //float lerp = accum / fixedDeltaTime;
        gameCode.api.render(&memory, 1);

        if(updateRan) {
            platform.input = {};
        }

        if(capFramerate) {
            LARGE_INTEGER workCounter = getTimestamp();
            double workDelta = getTimeElapsed(frameTime, workCounter, platform.timerFrequency);

            //TODO: Implement Vsync switch
            //! This should only happen if VSync is off
            if(workDelta < targetFrameTime) {
                Sleep((int)((targetFrameTime - workDelta) * 1000 - 1));
            }
            while(workDelta < targetFrameTime) {
                workCounter = getTimestamp();
                workDelta = getTimeElapsed(frameTime, workCounter, platform.timerFrequency);
            }
        }

        SwapBuffers(dc);

#if 0
        printf_s("last frame: %f ms\n", deltaTime * 1000);
        printf_s("FPS: %f\n", 1 / deltaTime);
        fflush(stdout);
#endif
    }

    return 0;
}