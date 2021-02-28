#ifndef MATH_H
#define MATH_H

template<typename T>
inline T max(T a, T b) {
    return a > b ? a : b;
}

template<typename T>
inline T min(T a, T b) {
    return a < b ? a : b;
}

template<typename T>
inline T clamp(T to_clamp, T min_value, T max_value) {
    return max(min(to_clamp, max_value), min_value);
}

#endif