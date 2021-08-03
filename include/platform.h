#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include "Windows.h"
#endif

#include "strings.h"
#include "vector.h"

struct Event;

struct OSWindow {
    //@Cleanup make this integer vectors and remove all the unnecessary casting everywhere
    Vec2 position;
    Vec2 size;
    
    //@Cleanup These should probably be state flags
    bool focused;
    bool fullscreen;
    bool borderless;
};

extern Array<Event> events;
extern Vec2 mouse_delta;

OSWindow* os_create_window(int width, int height, String title = ""_s);
u64 os_get_timestamp();
float os_elapsed_time(u64 from_stamp, u64 to_stamp);
//s64 os_get_timer_frequency();
void os_poll_events();
//void os_lock_mouse(OSWindow* window);
void os_set_mouse_center(OSWindow* window);
void os_show_mouse(bool show);
//bool os_is_fullscreen(OSWindow* window);
void os_set_fullscreen(OSWindow* window, bool fullscreen, bool borderless);
String os_read_entire_file(String path);

#endif