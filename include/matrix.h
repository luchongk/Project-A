#ifndef MATRIX_H
#define MATRIX_H

#include <cmath>

#include "maths.h"
#include "vector.h"

union Matrix {
    static const Matrix ident;

    float elems[16];
    Vec4 rows[4];
};

const Matrix Matrix::ident{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

inline Vec4 operator*(const Matrix& m, const Vec4& v) {
    Vec4 result;
    result.x = dot(m.rows[0], v);
    result.y = dot(m.rows[1], v);
    result.z = dot(m.rows[2], v);
    result.w = dot(m.rows[3], v);
    
    return result;
}

inline Vec3 operator*(const Matrix& m, const Vec3& v) {
    Vec3 result;
    result.x = dot(m.rows[0], vec4(v, 1));
    result.y = dot(m.rows[1], vec4(v, 1));
    result.z = dot(m.rows[2], vec4(v, 1));
    
    return result;
}

inline Matrix operator*(const Matrix& m, const Matrix& n) {
    Vec4 col1{n.elems[0], n.elems[4], n.elems[8],  n.elems[12]};
    Vec4 col2{n.elems[1], n.elems[5], n.elems[9],  n.elems[13]};
    Vec4 col3{n.elems[2], n.elems[6], n.elems[10], n.elems[14]};
    Vec4 col4{n.elems[3], n.elems[7], n.elems[11], n.elems[15]};
    
    return Matrix {
        dot(m.rows[0], col1), dot(m.rows[0], col2), dot(m.rows[0], col3), dot(m.rows[0], col4),
        dot(m.rows[1], col1), dot(m.rows[1], col2), dot(m.rows[1], col3), dot(m.rows[1], col4),
        dot(m.rows[2], col1), dot(m.rows[2], col2), dot(m.rows[2], col3), dot(m.rows[2], col4),
        dot(m.rows[3], col1), dot(m.rows[3], col2), dot(m.rows[3], col3), dot(m.rows[3], col4),
    };;
}

Matrix from_columns(Vec3 cols[3]) {
    return Matrix{
        cols[0].x, cols[1].x, cols[2].x, 0,
        cols[0].y, cols[1].y, cols[2].y, 0,
        cols[0].z, cols[1].z, cols[2].z, 0,
                0,         0,         0, 1,
    };
}

inline Vec3 position(const Matrix& m) {
    return Vec3{m.elems[3], m.elems[7], m.elems[11]};
}

inline Vec3 forward(const Matrix& m) {
    Vec3 v{m.elems[2], m.elems[6], m.elems[10]};
    
    return normalize(v);
}

inline Matrix translation(Vec3 v) {
    return Matrix{
        1, 0, 0, v.x,
        0, 1, 0, v.y,
        0, 0, 1, v.z,
        0, 0, 0,   1
    };
}

Matrix translate(const Matrix& m, Vec3 v) {
    Matrix result = m;
    result.rows[0].w += v.x;
    result.rows[1].w += v.y;
    result.rows[2].w += v.z;

    return result;
}

Matrix scale(const Matrix& m, float s) {
    return Matrix{
        s * m.elems[0], s * m.elems[1],   s * m.elems[2],  m.elems[3],
        s * m.elems[4], s * m.elems[5],   s * m.elems[6],  m.elems[7],
        s * m.elems[8], s * m.elems[9],  s * m.elems[10], m.elems[11],
           m.elems[12],    m.elems[13],      m.elems[14], m.elems[15],
    };
}

Matrix rotate(const Matrix&m, Vec3 v, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    float c_m = 1 - c;
    float xyc_m = v.x * v.y * c_m;
    float xzc_m = v.x * v.z * c_m;
    float yzc_m = v.y * v.z * c_m;
    float xs = v.x * s;
    float ys = v.y * s;
    float zs = v.z * s;
    
    Matrix rotation{
        c + v.x * v.x * c_m,          xyc_m - zs,          xzc_m + ys, 0,
                 xyc_m + zs, c + v.y * v.y * c_m,          yzc_m - xs, 0,
                 xzc_m - ys,          yzc_m + xs, c + v.z * v.z * c_m, 0,
                          0,                   0,                   0, 1,
    };

    return m * rotation;
}

Matrix rotation(float yaw, float pitch) {
    float cy = cos(yaw);
    float sy = sin(yaw);
    float cp = cos(pitch);
    float sp = sin(pitch);
    float sysp = sy * sp;
    float sycp = sy * cp;
    
    return Matrix{
         cy,      sysp,      sycp, 0,
          0,        cp,       -sp, 0,
        -sy,   cy * sp,   cy * cp, 0,
          0,         0,         0, 1
    };
}

//This name sucks. Should be something along the line of "to_view_coordinates" or something like that
Matrix lookDir(Vec3 pos, Vec3 forward, Vec3 up) {
    Vec3 right = normalize(cross(up, forward));
    up = cross(forward, right);

    return Matrix{
          right.x,   right.y,   right.z, dot(right, -pos),
             up.x,      up.y,      up.z, dot(up, -pos),
        forward.x, forward.y, forward.z, dot(forward, -pos),
                0,         0,         0, 1,
    };
}

//This name sucks. Should be something along the line of "to_view_coordinates" or something like that
Matrix lookAt(Vec3 pos, Vec3 to, Vec3 up) {
    Vec3 forward = normalize(to - pos);

    return lookDir(pos, forward, up);
}

Matrix perspective(int width, int height) {
    // @Journey 04/13/2021: We learned that Direct3D NDC coordinates go from 0 (near plane) to 1 (far plane) in the z-axis.
    // This means we can't rely on glm::perspective to create a projection matrix for us because OpenGL's convention
    // is to map z coordinates to values between -1 (near plane) and 1 (far plane). So.. we just learned how to make our own projection matrix!
    // X and Y axis are treated the same in D3D vs OpenGL (-1 to 1), so we borrowed those rows from glm's matrix.

    float aspect = (float)width / height;
    float tanHalfFovy = tan(radians(45.0f) / 2);
    return Matrix{
        1.0f / (aspect * tanHalfFovy),               0.0f,          0.0f,                0.0f,
                                 0.0f, 1.0f / tanHalfFovy,          0.0f,                0.0f,
                                 0.0f,               0.0f, -100.0f/99.9f, -100.0f * 0.1f/99.9f,
                                 0.0f,               0.0f,            -1,                0.0f
    };
}

#endif