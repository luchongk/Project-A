#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include "Windows.h"
#endif

#include "strings.h"
#include "vector.h"

struct OSWindow;

struct OSKeyboardEvent {
    u32 code;       //@Temporary: We are currently using windows key codes. Make codes cross platform!
    bool pressed;
};

struct OSTextEvent {

};

enum OSEventType {

};

struct OSEvents {
    Array<OSKeyboardEvent> keyboard;
    Array<OSTextEvent> chars;
    //Array<OSEventType> other;
    Vector2 mouse_delta;
};

OSEvents os_events;
bool os_keyboard[256];

OSWindow* os_create_window(int x, int y, int width, int height);
inline s64 os_get_timestamp();
inline s64 os_get_timer_frequency();
bool os_poll_events(OSWindow* window);
void os_lock_cursor(OSWindow* window);
bool os_is_fullscreen(OSWindow* window);
void os_set_fullscreen(OSWindow* window, bool fullscreen, bool borderless);
void os_swap_buffers(OSWindow* window);
String os_read_entire_file(String path, bool null_terminated = false);

#endif