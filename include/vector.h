#ifndef VECTOR_H
#define VECTOR_H

#include "glm/glm.hpp"

struct Vector2 {
    float x;
    float y;
};

struct Vector3I {
    //REFLECT
    
    int x;
    int y;
    int z;

    Vector3I() = default;
    Vector3I(int x, int y, int z) : x{x}, y{y}, z{z} {

    }

    inline Vector3I operator*(const int& scalar) const {
        return Vector3I{this->x * scalar, this->y * scalar, this->z * scalar};
    }

    inline Vector3I operator/(const int& scalar) const {
        return Vector3I{this->x / scalar, this->y / scalar, this->z / scalar};
    }

    inline Vector3I operator+=(const Vector3I& other) {
        this->x += other.x;
        this->y += other.y;
        this->z += other.z;

        return *this;
    }
    
    inline Vector3I operator+(const Vector3I& other) const {
        Vector3I temp{*this};
        return temp += other;
    }

    inline int dot(const Vector3I& other) {
        return this->x * other.x + this->y * other.y + this->z * other.z;
    }
};

inline Vector3I operator*(const int& scalar, const Vector3I& vec) {
    return vec * scalar;
}

inline Vector3I operator/(const int& scalar, const Vector3I& vec) {
    return vec / scalar;
}

struct Vector3 {
    //REFLECT
    
    float x;
    float y;
    float z;

    Vector3() = default;
    Vector3(float x, float y, float z) : x{x}, y{y}, z{z} { }
    Vector3(Vector3I v) : x{(float)v.x}, y{(float)v.y}, z{(float)v.z} { }

    inline Vector3 operator*(const float& scalar) const {
        return Vector3{this->x * scalar, this->y * scalar, this->z * scalar};
    }

    inline Vector3 operator/(const float& scalar) const {
        return Vector3{this->x / scalar, this->y / scalar, this->z / scalar};
    }

    inline Vector3 operator+=(const Vector3& other) {
        this->x += other.x;
        this->y += other.y;
        this->z += other.z;

        return *this;
    }
    
    inline Vector3 operator+(const Vector3& other) const {
        Vector3 temp{*this};
        return temp += other;
    }

    inline float dot(const Vector3& other) {
        return this->x * other.x + this->y * other.y + this->z * other.z;
    }

    inline Vector3 cross(const Vector3& other) {
        return Vector3 {
            y * other.z - other.y * z,
			z * other.x - other.z * x,
			x * other.y - other.x * y
        };
    }

    inline void normalize() {
        const float invsquare = 1 / this->dot(*this);
        
        this->x *= invsquare;
        this->y *= invsquare;
        this->z *= invsquare;
    }
    
    inline Vector3 normalized() {
        return Vector3{*this / (float)sqrt(this->dot(*this))};
    }

    inline glm::vec3 toGLM() {
        return glm::vec3{this->x, this->y, this->z};
    }
};

inline Vector3 operator*(const float& scalar, const Vector3& vec) {
    return vec * scalar;
}

inline Vector3 operator/(const float& scalar, const Vector3& vec) {
    return vec / scalar;
}

//-----------------------------

Vector2 vec2_parse(String s) {
    Vector2 v;
    v.x = (float)atof((const char*)eat_until(' ', &s).data);
    s = advance(s, 1);
    v.y = (float)atof((const char*)eat_until(' ', &s).data);

    return v;
}

Vector3 vec3_parse(String s) {
    Vector3 v;
    v.x = (float)atof((const char*)eat_until(' ', &s).data);
    s = advance(s, 1);
    v.y = (float)atof((const char*)eat_until(' ', &s).data);
    s = advance(s, 1);
    v.z = (float)atof((const char*)eat_until(' ', &s).data);

    return v;
}

#endif