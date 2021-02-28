#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
    #include "Windows.h"
#endif

#include "strings.h"
#include "vector.h"

struct OSWindow {
    bool focused;
    void* platform_data;
};

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

extern OSEvents os_events;

OSWindow* os_create_window();
inline s64 os_get_timestamp();
inline s64 os_get_timer_frequency();
bool os_poll_events();
void os_toggle_fullscreen(OSWindow* window, bool borderless);
void os_swap_buffers(OSWindow* window);
String os_read_entire_file(String path, bool null_terminated = false);

#endif