#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>
#include "utils.h"

struct PlatformAPI {
    size_t (*get_file_size)(const char* path);
    void (*read_entire_file)(const char* path, size_t file_size, void* file_data);
};

struct GameMemory {
    void* data;
    size_t data_size;
};

struct PlayerInput {
    bool pause;
    bool reset;
    int horizontal;
    int vertical;
    int mouseDeltaX;
    int mouseDeltaY;
};

#endif