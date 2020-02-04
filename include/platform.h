#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef DEBUG
#define assert(condition) if(!(condition)) (*(int*)nullptr) = 0
#else
#define assert(condition)
#endif

struct GameMemory;

typedef void startFunction(GameMemory*);
static void startStub(GameMemory*) {}
typedef void updateFunction(GameMemory*);
static void updateStub(GameMemory*) {}
typedef void renderFunction();
static void renderStub() {}

struct GameAPI {
    startFunction* start;
    updateFunction* update;
    renderFunction* render;
};

template<class T, size_t N>
constexpr size_t length(T (&)[N]) { return N; }

#endif