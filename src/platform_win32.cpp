#include <cstdio>
#include "Windows.h"

#include "types.h"
#include "glad/glad.h"

struct OSWindow {
    HWND handle;
    HDC device;
    WINDOWPLACEMENT placement;
};

static HINSTANCE app_instance;
static wchar_t window_class_name[] = L"Project_A";
static OSEvents os_events;

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

static OSWindow* os_create_window() {
    HWND hwnd = CreateWindow(window_class_name, L"Project_A", WS_OVERLAPPEDWINDOW,
                                200, 100, 1600, 800,
                                nullptr, nullptr, app_instance, nullptr);
                                
    if(!hwnd)
        return nullptr;

    HDC dc = GetDC(hwnd);

    if(!initOpenGL(dc))
        return nullptr;

    ShowWindow(hwnd, 5);

    OSWindow* window = alloc_<OSWindow>();
    window->handle = hwnd;
    window->device = dc;
    window->placement.length = sizeof(WINDOWPLACEMENT);
    return window;
}

inline static s64 os_get_timestamp() {
    LARGE_INTEGER timestamp;
    QueryPerformanceCounter(&timestamp);
    
    return timestamp.QuadPart;
}

inline static s64 os_get_timer_frequency() {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    return frequency.QuadPart;
}

static void clear_event_queue() {
    os_events.keyboard.count = 0;
    os_events.mouse_delta = {0,0};
}

static bool os_poll_events() {
    clear_event_queue();

    MSG msg{};
    while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if(msg.message == WM_QUIT) {
            return true;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return false;
}

static void os_toggle_fullscreen(OSWindow* window, bool borderless) {
    HWND hwnd = window->handle;

    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    if (dwStyle & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFOEX mi;
        mi.cbSize = sizeof(mi);
        if (GetWindowPlacement(hwnd, &window->placement) &&
            GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
        {
            ShowCursor(false);
            SetWindowLong(hwnd, GWL_STYLE,
                            dwStyle & ~WS_OVERLAPPEDWINDOW);
            
            SetWindowPos(hwnd, borderless ? HWND_TOP : HWND_TOPMOST,
                        mi.rcMonitor.left, mi.rcMonitor.top,
                        mi.rcMonitor.right - mi.rcMonitor.left,
                        mi.rcMonitor.bottom - mi.rcMonitor.top,
                        SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        ShowCursor(true);
        SetWindowLong(hwnd, GWL_STYLE,
                    dwStyle | WS_OVERLAPPEDWINDOW);

        SetWindowPlacement(hwnd, &window->placement);
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE |
                    SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

static void os_swap_buffers(OSWindow* window) {
    SwapBuffers(window->device);
}

static char* os_read_entire_file(const char* path) {
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    assert(file != INVALID_HANDLE_VALUE);

    uint file_size = GetFileSize(file, nullptr);
    char* file_data = alloc_<char>(file_size + 1);

    DWORD bytesRead;
    assert(ReadFile(file, file_data, (DWORD)file_size, &bytesRead, nullptr));
    assert(file_size == bytesRead);

    CloseHandle(file);

    ((uint8_t*)file_data)[file_size] = '\0';

    return file_data;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
        } break;

        case WM_SIZE: {
            glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
        } break;

        case WM_KEYUP: {
            uint32_t VKCode = (uint32_t)wParam;
            bool wasDown = lParam & 0x40000000;

            OSKeyboardEvent event{VKCode, false, wasDown};
            array_add(&os_events.keyboard, event);
        } break;

        case WM_KEYDOWN: {
            uint32_t VKCode = (uint32_t)wParam;
            bool wasDown = lParam & 0x40000000;

            OSKeyboardEvent event{VKCode, true, wasDown};
            array_add(&os_events.keyboard, event);
        } break;

#if 1
        /* Mouse detection via driver */
        case WM_INPUT: {
            UINT dwSize = sizeof(RAWINPUT);
            RAWINPUT raw;
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &dwSize, sizeof(RAWINPUTHEADER));

            if (raw.header.dwType == RIM_TYPEMOUSE) 
            {
                os_events.mouse_delta.x += raw.data.mouse.lLastX;
                os_events.mouse_delta.y += raw.data.mouse.lLastY;
            }

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

void main();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR lpCmdLine, int nCmdShow) {
    app_instance = hInstance;
    
    WNDCLASS wc{};
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = window_class_name;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    SetCursor(wc.hCursor);
    wc.style = CS_OWNDC;
    
    if(!RegisterClass(&wc))
        return 1;

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

    main();

    return 0;
    
/*
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
            
            //We dont want the time we took to reload to count as game_time, so we skip it.
            frameTime = getTimestamp();
        }

        quit = !pollEvents(&platform.input);

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
        
        //* RENDERING 
        //TODO: Stuttering comes from not doing position/rotation interpolation. Do that.
        //float lerp = accum / fixedDeltaTime;
        gameCode.api.render(&memory, 1);

        if(updateRan) {
            platform.input = {};
        }

        LARGE_INTEGER workCounter = getTimestamp();
        double workDelta = getTimeElapsed(frameTime, workCounter, platform.timerFrequency);
        if(capFramerate) {
            //TODO: Implement Vsync switch
            //! This should only happen if VSync is off
            if(workDelta < targetFrameTime) {
                Sleep((int)((targetFrameTime - workDelta) * 1000) - 1);
            }
            while(workDelta < targetFrameTime) {
                workCounter = getTimestamp();
                workDelta = getTimeElapsed(frameTime, workCounter, platform.timerFrequency);
            }
        }

        SwapBuffers(dc);
    }
*/
}