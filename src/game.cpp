#include <stdio.h>

#define DLLEXPORT extern "C" __declspec(dllexport)

DLLEXPORT void update() {
    //printf("So happy right now!\n");
}