#include <iostream>
#include <cstdio>
#include "iostream"
#include "strings.h"
#include "graphics.h"
#include "input.h"

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

static wchar_t window_class_name[] = L"Project_A";
static int default_window_style = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
static OSWindow* cursor_locked_to = nullptr;

OSWindow* os_create_window(int x, int y, int width, int height) {
    RECT wr = {0, 0, width, height};                    // set the size, but not the position
    AdjustWindowRect(&wr, default_window_style, false); // adjust the size

    HINSTANCE app_instance = GetModuleHandleA(nullptr);
    HWND hwnd = CreateWindow(window_class_name, L"Project_A", default_window_style,
                                x, y,
                                wr.right - wr.left,
                                wr.bottom - wr.top,
                                nullptr, nullptr, app_instance, nullptr);
                                
    if(!hwnd)
        return nullptr;

    if(!init_graphics(hwnd))
        return nullptr;

    OSWindow* window = alloc_<OSWindow>();
    *window = {
        hwnd,
        GetDC(hwnd),
        {(float)x, (float)y},
        {(float)width, (float)height},
        false,
        false,
        true,
    };

    SetProp(hwnd, L"window_data", window);
    
    ShowWindow(hwnd, SW_SHOW);
    
    return window;
}

s64 os_get_timestamp() {
    LARGE_INTEGER timestamp;
    QueryPerformanceCounter(&timestamp);
    
    return timestamp.QuadPart;
}

s64 os_get_timer_frequency() {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    return frequency.QuadPart;
}

bool os_poll_events(OSWindow* window) {
    input_next_frame();

    MSG msg{};
    while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if(msg.message == WM_QUIT) {
            end_graphics();
            return true;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
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

void os_lock_cursor(OSWindow* window) {
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
            //bool was_down = lParam & 0x40000000;

            //if(was_down) {
                //OSKeyboardEvent event{vk_code, false};
                //array_add(&os_events.keyboard, event);

                input.keys[vk_code].current = false;
            //}
        } break;

        case WM_KEYDOWN: {
            u32 vk_code = (u32)wParam;
            //bool was_down = lParam & 0x40000000;

            //if(!was_down) {
                //OSKeyboardEvent event{vk_code, true};
                //array_add(&os_events.keyboard, event);

                input.keys[vk_code].current = true;
            //}
        } break;

        case WM_NCLBUTTONDOWN: {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;

        case WM_ACTIVATE: {
            if(LOWORD(wParam) == WA_INACTIVE) {
                window->focused = false;
                
                clear_keys();

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
            input.mouse_delta = {0,0};

            clear_keys();
        } break;

        case WM_INPUT: {
            UINT dwSize = sizeof(RAWINPUT);
            RAWINPUT raw;
            
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &dwSize, sizeof(RAWINPUTHEADER));

            if (raw.header.dwType == RIM_TYPEMOUSE) 
            {
                input.mouse_delta.x += raw.data.mouse.lLastX;
                input.mouse_delta.y += raw.data.mouse.lLastY;
            }

            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void main();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc{};
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = window_class_name;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.style = CS_OWNDC;
    
    if(!RegisterClass(&wc))
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

    main();

    return 0;
}