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

OSWindow* os_create_window(int x, int y, int width, int height);
s64 os_get_timestamp();
s64 os_get_timer_frequency();
bool os_poll_events(OSWindow* window);
void os_lock_cursor(OSWindow* window);
bool os_is_fullscreen(OSWindow* window);
void os_set_fullscreen(OSWindow* window, bool fullscreen, bool borderless);
void os_swap_buffers(OSWindow* window);
String os_read_entire_file(String path, bool null_terminated = false);

#endif