#include <iostream>
#include <cstdio>
#include "iostream"
#include "strings.h"
#include "graphics.h"
#include "input.h"
#include "memory.h"

struct Win32Window {
    OSWindow common;
    HWND handle;
    HDC device;
};

static char window_class_name[] = "Project_A";
static int default_window_style = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
static bool mouse_visible = true;

static LARGE_INTEGER timer_frequency;

Array<Event> events;
//Vec2 mouse_position;
//Vec2 mouse_delta;

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
                                
    if(!hwnd) return nullptr;

    if(!init_graphics(hwnd)) return nullptr;

    Win32Window* window = alloc_<Win32Window>();
    window->common = {
        200, 100,
        (float)width, (float)height,
        false,
        false,
        true,
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
void os_poll_events() {
    player_input.mouse_delta = {0,0};

    array_reset(&events);
    
    MSG msg{};
    while(PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
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

void os_set_mouse_center(OSWindow* window) {
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

void os_set_fullscreen(OSWindow* window, bool fullscreen, bool borderless) {
    HWND hwnd = ((Win32Window*)window)->handle;
    
    if(fullscreen && !window->fullscreen) {
        window->fullscreen = true;
        
        if (borderless) {
            window->borderless = true;

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
        } else {
            window->borderless = false;
            set_fullscreen(true);
        }
    }
    else if(!fullscreen && window->fullscreen) {
        window->fullscreen = false;

        if(window->borderless) {
            SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE | WS_CLIPSIBLINGS | default_window_style);

            RECT wr = {0, 0, (int)window->size.x, (int)window->size.y};                    // set the size, but not the position
            AdjustWindowRect(&wr, default_window_style, false); // adjust the size

            SetWindowPos(hwnd, HWND_NOTOPMOST,
                            (int)window->position.x, (int)window->position.y,
                            wr.right - wr.left, wr.bottom - wr.top,
                            SWP_FRAMECHANGED);
        }
        else {
            set_fullscreen(false);
        }
    }
}

String os_read_entire_file(String path) {
    HANDLE file = CreateFileA((char*)path.data, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    assert(file != INVALID_HANDLE_VALUE);

    uint alloc_size = GetFileSize(file, nullptr);
    
    String content = new_string(alloc_size);

    DWORD bytesRead;
    assert(ReadFile(file, content.data, (DWORD)alloc_size, &bytesRead, nullptr));
    assert(alloc_size == bytesRead);

    CloseHandle(file);

    return content;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    OSWindow* window = (OSWindow*)GetPropA(hwnd, "window_data");

    switch(uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
        } break;

        case WM_SIZE: {
            Event e;
            e.type = EventType::WINDOW;
            e.window.window = window;
            
            e.window.type = EventWindowType::RESIZE;
            e.window.data1 = LOWORD(lParam);
            e.window.data2 = HIWORD(lParam);
            array_add(&events, e);
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
            array_add(&events, e);
        } break;

        case WM_KEYDOWN: {
            u32 vk_code = (u32)wParam;
            bool was_down = lParam & 0x40000000;

            Event e;
            e.type = EventType::KEY;
            e.key.keycode   = vk_code;
            e.key.pressed   = true;
            e.key.is_repeat = was_down;
            array_add(&events, e);
        } break;

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
            }
            
            array_add(&events, e);
        } break;

        case WM_EXITSIZEMOVE: {
            player_input.mouse_delta = {0,0};
        } break;

        case WM_MOUSEMOVE: {
            //@Temporary: We are casting mouse positions to float until we have a integer Vector type.
            player_input.mouse_pos_screen.x = (short)(lParam & 0xFFFF);
            player_input.mouse_pos_screen.y = (short)(lParam >> 16);
        } break;

        case WM_LBUTTONDOWN: {
            Event e;
            e.type = EventType::KEY;
            e.key.keycode   = VK_LBUTTON;
            e.key.pressed   = true;
            e.key.is_repeat = false;
            array_add(&events, e);
        } break;

        case WM_LBUTTONUP: {
            Event e;
            e.type = EventType::KEY;
            e.key.keycode   = VK_LBUTTON;
            e.key.pressed   = false;
            e.key.is_repeat = false;
            array_add(&events, e);
        } break;

        case WM_RBUTTONDOWN: {

        } break;

        case WM_RBUTTONUP: {

        } break;

        case WM_INPUT: {
            UINT dwSize = sizeof(RAWINPUT);
            RAWINPUT raw;
            
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &dwSize, sizeof(RAWINPUTHEADER));

            if(raw.header.dwType == RIM_TYPEMOUSE) 
            {
                player_input.mouse_delta.x += raw.data.mouse.lLastX;
                player_input.mouse_delta.y += raw.data.mouse.lLastY;
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