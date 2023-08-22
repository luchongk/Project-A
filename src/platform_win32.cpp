#include <iostream>
#include <cstdio>
#include "platform.h"
#include "iostream"
#include "strings.h"
#include "graphics.h"
#include "input.h"
#include "memory.h"
#include "timeapi.h"

struct Win32Window {
    OSWindow common;
    HWND handle;
    HDC device;
};

static char window_class_name[]  = "Project_A";
static int  default_window_style = WS_OVERLAPPEDWINDOW;
static bool mouse_visible        = true;

// These are used during the sizing of a window (as in clicking and dragging the borders) to keep track of some state that Windows™ doesn't want to keep track of for us.
static bool  in_sizemove = false;
static float desired_width;
static float desired_height;

static LARGE_INTEGER timer_frequency;

Array<Event> events;

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
                                
    if(!hwnd) return nullptr;

    //@Cleanup: Graphics should not be a dependency of platform code!
    if(!init_graphics(hwnd)) return nullptr;

    Win32Window* window = alloc_<Win32Window>();
    window->common = {
        200, 100,
        (float)width, (float)height,
        false,
        false,
    };
    window->handle = hwnd;
    window->device = GetDC(hwnd);

    SetPropA(hwnd, "window_data", window);
    
    ShowWindow(hwnd, SW_SHOW);
    
    return (OSWindow*)window;
}

u64 os_get_timestamp() {
    LARGE_INTEGER timestamp;
    QueryPerformanceCounter(&timestamp);
    
    return timestamp.QuadPart;
}

float os_elapsed_time(u64 from_stamp, u64 to_stamp) {
    return (to_stamp - from_stamp) / (float)timer_frequency.QuadPart;
}

//? Should we move this to input.h?
void os_poll_events(OSWindow* window) {
    input.mouse_delta_pixels = {0,0};

    array_reset(&events);
    
    MSG msg{};
    while(true) {
        bool has_message = PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE);
        if(!has_message) break;

        TranslateMessage(&msg);
        DispatchMessageA(&msg);

        if(msg.message == WM_QUIT) {
            Event e;
            e.type = EventType::QUIT;
            array_add(&events, e);
        }
    }
}

void os_show_mouse(bool show) {
    if(show != mouse_visible) {
        ShowCursor(show);
        mouse_visible = show;
    }
}

void os_set_mouse_to_center(OSWindow* window) {
    HWND hwnd = ((Win32Window*)window)->handle;
        
    RECT rect;
    GetClientRect(hwnd, &rect);

    POINT center;
    center.x = rect.right / 2;
    center.y = rect.bottom / 2;
    ClientToScreen(hwnd, &center);
    SetCursorPos(center.x, center.y);
}

void os_minimize_window(OSWindow* window) {
    Win32Window* win32_window = (Win32Window*)window;
    ShowWindow(win32_window->handle, SW_MINIMIZE);
}

void os_restore_window(OSWindow* window) {
    Win32Window* win32_window = (Win32Window*)window;
    ShowWindow(win32_window->handle, SW_RESTORE);
}

void os_set_fullscreen(OSWindow* window, bool fullscreen) {
    HWND hwnd = ((Win32Window*)window)->handle;
    
    if(fullscreen && !window->fullscreen) {
        window->fullscreen = true;

        MONITORINFOEX mi;
        mi.cbSize = sizeof(mi);
        if(GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE | WS_CLIPSIBLINGS);
            
            SetWindowPos(hwnd, HWND_TOP,
                            mi.rcMonitor.left, mi.rcMonitor.top,
                            mi.rcMonitor.right - mi.rcMonitor.left,
                            mi.rcMonitor.bottom - mi.rcMonitor.top,
                            SWP_FRAMECHANGED);

            RECT my_rect = {mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom};
            ClipCursor(&my_rect);
        }
    }
    else if(!fullscreen && window->fullscreen) {
        window->fullscreen = false;

        SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE | WS_CLIPSIBLINGS | default_window_style);

        RECT wr = {0, 0, (int)window->size.x, (int)window->size.y}; // set the size, but not the position
        AdjustWindowRect(&wr, default_window_style, false);         // adjust the size

        SetWindowPos(hwnd, HWND_NOTOPMOST,
                        (int)window->position.x, (int)window->position.y,
                        wr.right - wr.left, wr.bottom - wr.top,
                        SWP_FRAMECHANGED);

        ClipCursor(nullptr);
    }
}

bool os_read_entire_file(String path, Array<u8>* bytes) {
    HANDLE file = CreateFileA((char*)path.data, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if(file == INVALID_HANDLE_VALUE) return false;

    uint alloc_size = GetFileSize(file, nullptr);
    
    array_reserve(bytes, alloc_size);

    DWORD bytesRead;
    auto result = ReadFile(file, bytes->data, alloc_size, &bytesRead, nullptr);
    if(!result) return false;

    assert(alloc_size == bytesRead);
    bytes->count = alloc_size;

    CloseHandle(file);
    return true;
}

void os_write_file(String path, String to_write) {
    HANDLE file = CreateFileA((char*)path.data, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW, 0, nullptr);
    assert(file != INVALID_HANDLE_VALUE);

    DWORD bytesWritten;
    auto result = WriteFile(file, to_write.data, to_write.count, &bytesWritten, nullptr);
    assert(result);
    assert(to_write.count == bytesWritten);

    CloseHandle(file);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    OSWindow* window = (OSWindow*)GetPropA(hwnd, "window_data");

    switch(uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }

        case WM_SIZE: {
            //TODO: Handle window minimization.

            if(in_sizemove) {
                desired_width  = (float)LOWORD(lParam);
                desired_height = (float)HIWORD(lParam);
            }
            else {
                window->size.x = (float)LOWORD(lParam);
                window->size.y = (float)HIWORD(lParam);

                Event e;
                e.type = EventType::WINDOW;
                e.window.window = window;
                
                e.window.type = EventWindowType::RESIZE;
                array_add(&events, e);
            }
            
            break;
        }

        case WM_MOVE: {
            if(in_sizemove) {
                window->position.x = (float)LOWORD(lParam);
                window->position.y = (float)HIWORD(lParam);
            }
            break;
        }

        //This removes the weird beep sound when pressing ALT+Enter
        case WM_SYSCHAR: break;

        //Don't let ALT key freeze the window but still handle exiting with ALT+F4
        case WM_SYSKEYDOWN: {
            if(wParam == VK_F4) PostQuitMessage(0);
            break;
        }

        case WM_CHAR: {
            auto c = (char)wParam;

            // We are ignoring control code chars (U+0000 - U+001F) because it doesn't make any sense at ALL to receive them as chars.
            // If you want to support them in your application use the WM_KEYDOWN/WM_KEYUP events.
            if(c > 31) {
                Event e;
                e.type = EventType::TEXT;
                e.text.character = c;
                array_add(&events, e);
            }

            break;
        }

        case WM_KEYUP: {
            u32 vk_code = (u32)wParam;

            Event e;
            e.type = EventType::KEY;
            e.key.keycode   = vk_code;
            e.key.pressed   = false;
            e.key.is_repeat = false;
            array_add(&events, e);
            
            break;
        }

        case WM_KEYDOWN: {
            u32 vk_code = (u32)wParam;
            bool was_down = lParam & 0x40000000;

            Event e;
            e.type = EventType::KEY;
            e.key.keycode   = vk_code;
            e.key.pressed   = true;
            e.key.is_repeat = was_down;
            array_add(&events, e);
            
            break;
        }

        case WM_ACTIVATE: {
            Event e;
            e.type = EventType::WINDOW;
            e.window.window = window;

            if(LOWORD(wParam) == WA_INACTIVE) {
                e.window.type = EventWindowType::FOCUS_LOST;
                window->focused = false;
            }
            else {
                e.window.type = EventWindowType::FOCUS_GAINED;
                window->focused = true;
                
                if(window->fullscreen) {
                    RECT monitor = {0, 0, 1920, 1080};  //@Cleanup: Save monitor size for this.
                    ClipCursor(&monitor);
                }
            }
            
            array_add(&events, e);
            
            break;
        }

        case WM_ENTERSIZEMOVE: {
            in_sizemove = true;
            desired_width  = window->size.x;
            desired_height = window->size.y;
            break;
        }

        case WM_EXITSIZEMOVE: {
            in_sizemove = false;
            input.mouse_delta_pixels = {0,0};

            if(desired_width != window->size.x || desired_height != window->size.y) {
                window->size.x = desired_width;
                window->size.y = desired_height;

                Event e;
                e.type = EventType::WINDOW;
                e.window.window = window;
                
                e.window.type = EventWindowType::RESIZE;
                array_add(&events, e);
            }

            break;
        }

        case WM_MOUSEMOVE: {
            //@Temporary: We are casting mouse positions to float until we have an integer Vector type.
            input.mouse_pos_pixels.x = (short)(lParam & 0xFFFF);
            input.mouse_pos_pixels.y = (short)(lParam >> 16);
            
            break;
        }

        case WM_LBUTTONDOWN: {
            Event e;
            e.type = EventType::KEY;
            e.key.keycode   = VK_LBUTTON;
            e.key.pressed   = true;
            e.key.is_repeat = false;
            array_add(&events, e);
            
            break;
        }

        case WM_LBUTTONUP: {
            Event e;
            e.type = EventType::KEY;
            e.key.keycode   = VK_LBUTTON;
            e.key.pressed   = false;
            e.key.is_repeat = false;
            array_add(&events, e);
            
            break;
        }

        case WM_RBUTTONDOWN: {
            Event e;
            e.type = EventType::KEY;
            e.key.keycode   = VK_RBUTTON;
            e.key.pressed   = true;
            e.key.is_repeat = false;
            array_add(&events, e);
            
            break;
        }

        case WM_RBUTTONUP: {
            Event e;
            e.type = EventType::KEY;
            e.key.keycode   = VK_RBUTTON;
            e.key.pressed   = false;
            e.key.is_repeat = false;
            array_add(&events, e);
            
            break;
        }

        case WM_MOUSEWHEEL: {
            Event e;
            e.type = EventType::SCROLL;
            e.scroll.amount = (short)(wParam >> 16) / WHEEL_DELTA;
            array_add(&events, e);
            break;
        }

        case WM_INPUT: {
            UINT dwSize = sizeof(RAWINPUT);
            RAWINPUT raw;
            
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &dwSize, sizeof(RAWINPUTHEADER));

            if(raw.header.dwType == RIM_TYPEMOUSE) 
            {
                input.mouse_delta_pixels.x += raw.data.mouse.lLastX;
                input.mouse_delta_pixels.y -= raw.data.mouse.lLastY;
            }

            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
        }

        default:
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}

void main();
#include <strsafe.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR lpCmdLine, int nCmdShow) {
    setvbuf(stdout, nullptr, _IONBF, BUFSIZ);
    timeBeginPeriod(1);

    WNDCLASSA wc{};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = window_class_name;
    
    if(!RegisterClassA(&wc))
        return 1;

#if 1
    RAWINPUTDEVICE Rid;
    Rid.usUsagePage = 0x01;
    Rid.usUsage = 0x02;
    Rid.dwFlags = 0; //RIDEV_NOLEGACY;   // Ignores legacy mouse messages
    Rid.hwndTarget = nullptr;
    
    if (!RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE))) {
        /* ERROR */
    }
#endif

    QueryPerformanceFrequency(&timer_frequency);

    main();

    return 0;
}