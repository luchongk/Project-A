#include <iostream>
#include <cstdio>
#include "iostream"
#include "strings.h"
#include "graphics.h"
#include "input.h"
#include "memory.h"

struct OSWindow {
    HWND handle;
    HDC device;
    
    //@Cleanup make this integer vectors and remove all the unnecessary casting everywhere
    Vec2 position;
    Vec2 size;
    
    //@Cleanup These should probably be state flags
    bool focused;
    bool fullscreen;
    bool borderless;
};

static char window_class_name[] = "Project_A";
static int default_window_style = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
static OSWindow* cursor_locked_to = nullptr;

static LARGE_INTEGER timer_frequency;

OSWindow* os_create_window(int width, int height, String title) {
    RECT wr = {0, 0, width, height};                    // set the size, but not the position
    AdjustWindowRect(&wr, default_window_style, false); // adjust the size

    char* null_terminated_title;
    if(title.count > 0) {
        null_terminated_title = to_cstring(title);
    }
    else {
        null_terminated_title = window_class_name;
    }
    
    HINSTANCE app_instance = GetModuleHandleA(nullptr);
    HWND hwnd = CreateWindowA(window_class_name, null_terminated_title, default_window_style,
                                200, 100,
                                wr.right - wr.left,
                                wr.bottom - wr.top,
                                nullptr, nullptr, app_instance, nullptr);

    if(title.count > 0) {
        free_(null_terminated_title);
    }
                                
    if(!hwnd)
        return nullptr;

    if(!init_graphics(hwnd))
        return nullptr;

    OSWindow* window = alloc_<OSWindow>();
    *window = {
        hwnd,
        GetDC(hwnd),
        {0,0},
        {(float)width, (float)height},
        false,
        false,
        true,
    };

    SetProp(hwnd, L"window_data", window);
    
    ShowWindow(hwnd, SW_SHOW);
    
    return window;
}

float os_get_time() {
    LARGE_INTEGER timestamp;
    QueryPerformanceCounter(&timestamp);
    
    return timestamp.QuadPart / (float)timer_frequency.QuadPart;
}

/*s64 os_get_timer_frequency() {
    static LARGE_INTEGER frequency = 0;
    QueryPerformanceFrequency(&frequency);

    return frequency.QuadPart;
}*/

bool os_poll_events(OSWindow* window) {
    controls.mouse_delta = {0,0};

    MSG msg{};
    while(PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if(msg.message == WM_QUIT) {
            end_graphics();
            return true;
        }

        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    if(cursor_locked_to == window && window->focused) {
        HWND hwnd = window->handle;
        
        RECT rect;
        GetClientRect(hwnd, &rect);

        POINT center;
        center.x = rect.right / 2;
        center.y = rect.bottom / 2;
        ClientToScreen(hwnd, &center);
        SetCursorPos(center.x, center.y);
    }

    return false;
}

void os_lock_mouse(OSWindow* window) {
    ShowCursor(window ? false : true);
    cursor_locked_to = window;
}

bool os_is_fullscreen(OSWindow* window) {
    return window->fullscreen;
}

void os_set_fullscreen(OSWindow* window, bool fullscreen, bool borderless) {
    if(borderless) {
        HWND hwnd = window->handle;

        if (fullscreen && !window->fullscreen)
        {
            window->fullscreen = true;
            window->borderless = true;

            RECT rect;
            GetWindowRect(hwnd, &rect);
            window->position = {(float)rect.left, (float)rect.top};
            window->size = {(float)(rect.right - rect.left), (float)(rect.bottom - rect.top)};
            
            MONITORINFOEX mi;
            mi.cbSize = sizeof(mi);
            if (GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
                SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE | WS_CLIPSIBLINGS);
                
                SetWindowPos(hwnd, HWND_TOP,
                                mi.rcMonitor.left, mi.rcMonitor.top,
                                mi.rcMonitor.right - mi.rcMonitor.left,
                                mi.rcMonitor.bottom - mi.rcMonitor.top,
                                SWP_FRAMECHANGED);
            }
        } else if(!fullscreen && window->fullscreen) {
            window->fullscreen = false;

            SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE | WS_CLIPSIBLINGS | default_window_style);

            SetWindowPos(hwnd, HWND_NOTOPMOST,
                            (int)window->position.x, (int)window->position.y,
                            (int)window->size.x, (int)window->size.y,
                            SWP_FRAMECHANGED);
        }
    }
    else {
        window->fullscreen = fullscreen;
        window->borderless = false;
        set_fullscreen_state(fullscreen);
    }
}

void os_swap_buffers(OSWindow* window) {
    swap_buffers();
}

String os_read_entire_file(String path) {
    HANDLE file = CreateFileA((char*)path.data, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    assert(file != INVALID_HANDLE_VALUE);

    uint size = GetFileSize(file, nullptr);
    
    uint alloc_size = size;
    String content = new_string(alloc_size);

    DWORD bytesRead;
    assert(ReadFile(file, content.data, (DWORD)size, &bytesRead, nullptr));
    assert(size == bytesRead);

    CloseHandle(file);

    return content;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    OSWindow* window = (OSWindow*)GetProp(hwnd, L"window_data");

    switch(uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
        } break;

        case WM_SIZE: {
            adjust_size(LOWORD(lParam), HIWORD(lParam));
        } break;

        //Remove weird beep when ALT+Enter
        case WM_SYSCHAR: break;

        //Don't let ALT freeze the window but still handle exiting with ALT+F4
        case WM_SYSKEYDOWN: {
            if(wParam == VK_F4) PostQuitMessage(0);
        } break;

        case WM_KEYUP: {
            u32 vk_code = (u32)wParam;

            Event e;
            e.type = EventType::KEY;
            e.key.keycode   = vk_code;
            e.key.pressed   = false;
            e.key.is_repeat = false;

            handle_event(&e);
        } break;

        case WM_KEYDOWN: {
            u32 vk_code = (u32)wParam;
            bool was_down = lParam & 0x40000000;

            Event e;
            e.type = EventType::KEY;
            e.key.keycode   = vk_code;
            e.key.pressed   = true;
            e.key.is_repeat = was_down;

            handle_event(&e);
        } break;

        case WM_ACTIVATE: {
            if(LOWORD(wParam) == WA_INACTIVE) {
                window->focused = false;
                
                if(cursor_locked_to) ShowCursor(true);

                if(window->fullscreen && !window->borderless) {
                    set_fullscreen_state(false);
                    ShowWindow(hwnd, SW_MINIMIZE);
                }
            }
            else {
                window->focused = true;

                if(cursor_locked_to) ShowCursor(false);

                if(window->fullscreen && !window->borderless) {
                    ShowWindow(hwnd, SW_RESTORE);
                    set_fullscreen_state(true);
                }
            }
        } break;

        case WM_EXITSIZEMOVE: {
            controls.mouse_delta = {0,0};
        } break;

        case WM_INPUT: {
            UINT dwSize = sizeof(RAWINPUT);
            RAWINPUT raw;
            
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &dwSize, sizeof(RAWINPUTHEADER));

            if (raw.header.dwType == RIM_TYPEMOUSE) 
            {
                controls.mouse_delta.x += raw.data.mouse.lLastX;
                controls.mouse_delta.y += raw.data.mouse.lLastY;
            }

            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
        } break;

        default:
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void main();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc{};
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = window_class_name;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.style = CS_OWNDC;
    
    if(!RegisterClassA(&wc))
        return 1;

#if 1
    RAWINPUTDEVICE Rid;
    Rid.usUsagePage = 0x01; 
    Rid.usUsage = 0x02; 
    Rid.dwFlags = 0; //RIDEV_NOLEGACY;   // adds HID mouse and also ignores legacy mouse messages
    Rid.hwndTarget = 0;    

    if (!RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE))) {
        /* ERROR */
    }
#endif

    QueryPerformanceFrequency(&timer_frequency);

    main();

    return 0;
}