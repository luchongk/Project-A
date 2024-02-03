#ifndef MATH_H
#define MATH_H

#include "stdio.h"

const float PI  = 3.141592741f;
const float TAU = 6.28318548f;

template<typename T>
T max(T a, T b) {
    return a > b ? a : b;
}

template<typename T>
T min(T a, T b) {
    return a < b ? a : b;
}

template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
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

float exp_interpolate(float current, float target, float dt, float snappiness = 10) {
    auto difference = target - current;

    if(fabs(difference) < 0.001f) {
        return target;
    }
    
    auto increment = difference * snappiness * dt;
    // Prevent overshooting
    if(fabs(increment) - fabs(difference) > 0) {
        return target;
    }
    
    return current + increment;
}

#endif