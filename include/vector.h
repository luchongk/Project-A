#ifndef VECTOR_H
#define VECTOR_H

#include "glm/vec3.hpp"

/*struct Vector2 {

    float x;
    float y;
};

REFLECTION_REGISTRATION(Vector2) {
    CLASS->addField("x", &Vector2::x)
        ->addField("y", &Vector2::y);
}*/

struct Vector3 {
    //REFLECT
    
    float x;
    float y;
    float z;

    Vector3() = default;
    Vector3(float x, float y, float z) : x{x}, y{y}, z{z} {

    }

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
        return Vector3{*this / sqrt(this->dot(*this))};
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

/*REFLECTION_REGISTRATION(Vector3) {
    CLASS->addField("x", &Vector3::x)
        ->addField("y", &Vector3::y)
        ->addField("z", &Vector3::z);
}*/

#endif