#ifndef PLATFORM_H
#define PLATFORM_H

constexpr long long kilobytes(int n) { return 1024 * n; }
constexpr long long megabytes(int n) { return 1024 * kilobytes(n); }
constexpr long long gigabytes(int n) { return 1024 * megabytes(n); }
constexpr long long terabytes(int n) { return 1024 * gigabytes(n); }

typedef void startFunction(GameMemory*);
static void startStub(GameMemory*) {}
typedef void updateFunction(GameMemory*);
static void updateStub(GameMemory*) {}
typedef void renderFunction(GameMemory*);
static void renderStub(GameMemory*) {}

struct GameAPI {
    startFunction* start;
    updateFunction* update;
    renderFunction* render;
};

#endif