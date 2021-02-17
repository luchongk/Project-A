#ifndef MATH_H
#define MATH_H

inline int max(int a, int b) {
    return a > b ? a : b;
}

inline int min(int a, int b) {
    return a < b ? a : b;
}

inline int clamp(int to_clamp, int min_value, int max_value) {
    return max(min(to_clamp, max_value), min_value);
}

#endif