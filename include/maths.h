#ifndef MATH_H
#define MATH_H

const float PI = 3.141592741f;
const float TAU = 6.28318548f;

template<typename T>
T max(T a, T b) {
    return a > b ? a : b;
}

template<typename T>
T min(T a, T b) {
    return a < b ? a : b;
}

template<typename T>
T clamp(T to_clamp, T min_value, T max_value) {
    return max(min(to_clamp, max_value), min_value);
}

float normalize_angle(float angle) {
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