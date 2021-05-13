#ifndef MATH_H
#define MATH_H

const float PI = 3.141592741f;
const float TAU = 6.28318548f;

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

float clamp_angle(float angle) {
    if(angle > TAU) {
        angle -= TAU;
    }
    else if(angle < 0) {
        angle += TAU;
    }

    return angle;
}

constexpr float radians(float degrees) {
    return PI * degrees / 180;
}

#endif