#ifndef PRELOAD_MODULE
#define PRELOAD_MODULE

#include "base/platform.h"
#include "base/temp_allocator.h"

////////////////////////////////////////////////////////////
// This header file gets auto included in every .cpp file //
////////////////////////////////////////////////////////////

#if !defined(DEBUG) && !defined(ENABLE_ASSERTS)
#define NDEBUG
#endif

// Implicit initialization
//
// WARNING! This uses the same mechanism as static initialization and so it's prone to the whole static initilization order fiasco thing.
// It's the only thing in any modules using this mechanism so there's consistency in that, but beware if you plan to use static initialization.
// Static initialization is a bad idea anyways, but we're using it here so that the user already has some convenient stuff (like temp_storage)
// already available by the time main() gets called, otherwise the user would need to initialize them manually.

#if OS_WINDOWS
inline LARGE_INTEGER timer_frequency;
#endif

inline int implicit_initialization() {
    ensure_temp_allocator_init();

#if OS_WINDOWS
    QueryPerformanceFrequency(&timer_frequency);
#endif

    return 0;
}
inline int _dummy___var_ = implicit_initialization();

#endif