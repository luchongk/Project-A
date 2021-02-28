#include <iostream>
#include <cstdio>
#include "strings.h"

#include "glad/glad.h"

struct Win32Window {
    OSWindow window;
    HWND handle;
    HDC device;
    WINDOWPLACEMENT placement;
};

static HINSTANCE app_instance;
static wchar_t window_class_name[] = L"Project_A";
OSEvents os_events;

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

OSWindow* os_create_window() {
    HWND hwnd = CreateWindow(window_class_name, L"Project_A", WS_OVERLAPPEDWINDOW,
                                200, 100, 1600, 800,
                                nullptr, nullptr, app_instance, nullptr);
                                
    if(!hwnd)
        return nullptr;

    HDC dc = GetDC(hwnd);

    if(!initOpenGL(dc))
        return nullptr;

    Win32Window* window_data = alloc_<Win32Window>();
    window_data->window = {};
    window_data->window.platform_data = window_data;
    window_data->handle = hwnd;
    window_data->device = dc;
    window_data->placement.length = sizeof(WINDOWPLACEMENT);

    SetProp(hwnd, TEXT("window_data"), window_data);
    
    ShowWindow(hwnd, 5);
    
    return &window_data->window;
}

inline s64 os_get_timestamp() {
    LARGE_INTEGER timestamp;
    QueryPerformanceCounter(&timestamp);
    
    return timestamp.QuadPart;
}

inline s64 os_get_timer_frequency() {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    return frequency.QuadPart;
}

static void clear_event_queue() {
    os_events.mouse_delta = {0,0};
    os_events.keyboard.count = 0;
    os_events.chars.count = 0;
    //os_events.other.count = 0;
}

bool os_poll_events() {
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

void os_toggle_fullscreen(OSWindow* window, bool borderless) {
    Win32Window* window_data = (Win32Window*)window->platform_data;
    HWND hwnd = window_data->handle;

    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    if (dwStyle & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFOEX mi;
        mi.cbSize = sizeof(mi);
        if (GetWindowPlacement(hwnd, &window_data->placement) &&
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

        SetWindowPlacement(hwnd, &window_data->placement);
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE |
                    SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

void os_swap_buffers(OSWindow* window) {
    Win32Window* window_data = (Win32Window*)window->platform_data;
    
    SwapBuffers(window_data->device);
}

String os_read_entire_file(String path, bool null_terminated) {
    HANDLE file = CreateFileA((char*)path.data, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    assert(file != INVALID_HANDLE_VALUE);

    uint size = GetFileSize(file, nullptr);
    
    uint alloc_size = size;
    if(null_terminated) alloc_size++;
    String content = new_string(alloc_size);

    DWORD bytesRead;
    assert(ReadFile(file, content.data, (DWORD)size, &bytesRead, nullptr));
    assert(size == bytesRead);

    CloseHandle(file);

    if(null_terminated) content.data[size] = '\0';

    return content;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Win32Window* window_data = (Win32Window*)GetProp(hwnd, TEXT("window_data"));
    
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

            if(wasDown) {
                OSKeyboardEvent event{VKCode, false};
                array_add(&os_events.keyboard, event);
            }
        } break;

        case WM_KEYDOWN: {
            uint32_t VKCode = (uint32_t)wParam;
            bool wasDown = lParam & 0x40000000;

            if(!wasDown) {
                OSKeyboardEvent event{VKCode, true};
                array_add(&os_events.keyboard, event);
            }
        } break;

        case WM_ACTIVATE: {
            window_data->window.focused = wParam != 0;
        } break;

        case WM_EXITSIZEMOVE: {
            os_events.mouse_delta = {0,0};
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
}