#ifndef VECTOR_H
#define VECTOR_H

#include "math.h"
#include "strings.h"

//Vector2

union Vec2 {
    struct {
        float x;
        float y;
    };
    float elems[2];
};

Vec2 operator+(const Vec2& u, const Vec2& v) {
    return Vec2{u.x + v.x, u.y + v.y};
}

Vec2 operator-(const Vec2& u, const Vec2& v) {
    return Vec2{u.x - v.x, u.y - v.y};
}

Vec2 operator-(const Vec2& u) {
    return Vec2{-u.x, -u.y};
}

Vec2 operator+=(Vec2& u, const Vec2& v) {
    u = u + v;
    return u;
}

Vec2 operator-=(Vec2& u, const Vec2& v) {
    u = u - v;
    return u;
}

Vec2 operator*(const Vec2& u, const Vec2& v) {
    return Vec2{u.x * v.x, u.y * v.y};
}

Vec2 operator/(const Vec2& u, const Vec2& v) {
    return Vec2{u.x / v.x, u.y / v.y};
}

Vec2 operator*(const Vec2& v, const float& scalar) {
    return Vec2{v.x * scalar, v.y * scalar};
}

Vec2 operator*(const float& scalar, const Vec2& v) {
    return v * scalar;
}

Vec2 operator/(const Vec2& v, const float& scalar) {
    return Vec2{v.x / scalar, v.y / scalar};
}

float dot(const Vec2& u, const Vec2& v) {
    return u.x * v.x + u.y * v.y;
}

float length(Vec2 v) {
    return (float)sqrt(v.x * v.x + v.y * v.y);
}

Vec2 normalize(Vec2 v) {
    float v_length = length(v);
    if(v_length < 0.0001) return Vec2{};
    return v / v_length;
}

Vec2 parse_vec2(String s) {
    Vec2 v;
    v.x = (float)atof((const char*)eat_until(' ', &s).data);
    s = advance(s, 1);
    v.y = (float)atof((const char*)eat_until(' ', &s).data);

    return v;
}

//Vector3

union Vec3 {
    struct {
        Vec2 v;
        float z2;
    };
    struct {
        float x;
        float y;
        float z;
    };
    struct {
        float r;
        float g;
        float b;
    };
    float elems[3];
};

Vec3 operator+(const Vec3& u, const Vec3& v) {
    return Vec3{u.x + v.x, u.y + v.y, u.z + v.z};
}

Vec3 operator-(const Vec3& u, const Vec3& v) {
    return Vec3{u.x - v.x, u.y - v.y, u.z - v.z};
}

Vec3 operator-(const Vec3& v) {
    return Vec3{-v.x, -v.y, -v.z};
}

Vec3 operator+=(Vec3& u, const Vec3& v) {
    u = u + v;
    return u;
}

Vec3 operator-=(Vec3& u, const Vec3& v) {
    u = u - v;
    return u;
}

Vec3 operator*(const Vec3& u, const Vec3& v) {
    return Vec3{u.x * v.x, u.y * v.y, u.z * v.z};
}

Vec3 operator*(const Vec3& v, const float& scalar) {
    return Vec3{v.x * scalar, v.y * scalar, v.z * scalar};
}

Vec3 operator*(const float& scalar, const Vec3& v) {
    return v * scalar;
}

Vec3 operator/(const Vec3& v, const float& scalar) {
    return Vec3{v.x / scalar, v.y / scalar, v.z / scalar};
}

float dot(const Vec3& u, const Vec3& v) {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

Vec3 cross(Vec3 u, Vec3 v) {
    return Vec3 {
        u.y * v.z - v.y * u.z,
        u.z * v.x - v.z * u.x,
        u.x * v.y - v.x * u.y
    };
}

float length(Vec3 v) {
    return (float)sqrt(dot(v,v));
}

Vec3 normalize(Vec3 v) {
    float v_length = length(v);
    if(v_length < 0.0001) return Vec3{};
    return v / v_length;
}

Vec3 angles_to_vec(float yaw, float pitch) {
    Vec3 result;
    result.x = sinf(yaw) * cosf(pitch);
    result.y = sinf(pitch);
    result.z = cosf(yaw) * cosf(pitch);
    result = normalize(result);

    return result;
}

Vec3 parse_vec3(String s) {
    Vec3 v;
    v.x = (float)atof((const char*)eat_until(' ', &s).data);
    s = advance(s, 1);
    v.y = (float)atof((const char*)eat_until(' ', &s).data);
    s = advance(s, 1);
    v.z = (float)atof((const char*)eat_until(' ', &s).data);

    return v;
}

//Vector4

union Vec4 {
    struct {
        Vec3 v;
        float w;
    };
    struct {
        float x;
        float y;
        float z;
        float w;
    };
    struct {
        float r;
        float g;
        float b;
        float a;
    };
    float elems[4];
};

Vec4 operator+(const Vec4& u, const Vec4& v) {
    return Vec4{u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w};
}

Vec4 operator-(const Vec4& u, const Vec4& v) {
    return Vec4{u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w};
}

Vec4 operator+=(Vec4& u, const Vec4& v) {
    u = u + v;
    return u;
}

Vec4 operator*(const Vec4& u, const Vec4& v) {
    return Vec4{u.x * v.x, u.y * v.y, u.z * v.z, u.w * v.w};
}

Vec4 operator*(const Vec4& v, const float& scalar) {
    return Vec4{v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar};
}

Vec4 operator*(const float& scalar, const Vec4& v) {
    return v * scalar;
}

Vec4 operator/(const Vec4& v, const float& scalar) {
    return Vec4{v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar};
}

float dot(const Vec4& u, const Vec4& v) {
    return u.x * v.x + u.y * v.y + u.z * v.z + u.w * v.w;
}

float length(Vec4 v) {
    return (float)sqrt(dot(v,v));
}

Vec4 normalize(Vec4 v) {
    return v / length(v);
}

Vec4 parse_vec4(String s) {
    Vec4 v;
    v.x = (float)atof((const char*)eat_until(' ', &s).data);
    s = advance(s, 1);
    v.y = (float)atof((const char*)eat_until(' ', &s).data);
    s = advance(s, 1);
    v.z = (float)atof((const char*)eat_until(' ', &s).data);
    s = advance(s, 1);
    v.w = (float)atof((const char*)eat_until(' ', &s).data);

    return v;
}

//Convinient conversions

Vec2 vec2(const Vec3 v) {
    return Vec2{v.x, v.y};
}

Vec2 vec2(const Vec4 v) {
    return Vec2{v.x, v.y};
}

Vec3 vec3(const Vec4 v) {
    return Vec3{v.x, v.y, v.z};
}

Vec4 vec4(const Vec3 v, float w) {
    return Vec4{v.x, v.y, v.z, w};
}

#endif