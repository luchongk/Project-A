#ifndef PLATFORM_H
#define PLATFORM_H

#include "array.h"
#include "vector.h"

struct OSWindow;

struct OSKeyboardEvent {
    u32 code;       //@Temporary: We are currently using windows key codes. Make codes cross platform!
    bool pressed;
    bool wasDown;
};

struct OSTextEvent {

};

struct OSEvents {
    Array<OSKeyboardEvent> keyboard;
    Array<OSTextEvent> chars;
    Vector2 mouse_delta;
};

#endif