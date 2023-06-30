#ifndef UTILS_H
#define UTILS_H

#ifdef DEBUG
#undef assert
constexpr void assert(bool condition) { if(!(condition)) { int a = *(int*)0; a; } }
#else
#undef assert
constexpr void assert(bool condition) { }
#endif

// Enum class values don't get automatically casted to int like regular enum values do. We want that while still having the values behind a namespace, like enum class does.
// So this is basically a way to get the best of both worlds in a language as trash as this.
#define ENUM(name, ...) namespace name##_ { enum name { __VA_ARGS__ };}; using name = name##_::name;

template<class T, size_t N>
constexpr size_t length(T (&)[N]) { return N; }

constexpr long long kilobytes(int n) { return 1024 * (long long)n; }
constexpr long long megabytes(int n) { return 1024 * kilobytes(n); }
constexpr long long gigabytes(int n) { return 1024 * megabytes(n); }
constexpr long long terabytes(int n) { return 1024 * gigabytes(n); }

#endif