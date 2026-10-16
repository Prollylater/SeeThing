#ifndef CORE_GEOMETRY_H
#define CORE_GEOMETRY_H

#include <array>
#include <cmath>
#include <optional>

// Small fixed-size math: colour matrices, 3x3 kernels, 2D transforms. Image-sized data lives in Image<T>.
namespace core
{
    struct Vec3f
    {
        float x = 0, y = 0, z = 0;

        float &operator[](int i) { return i == 0 ? x : (i == 1 ? y : z); }
        float operator[](int i) const { return i == 0 ? x : (i == 1 ? y : z); }
        friend Vec3f operator+(Vec3f a, Vec3f b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
        friend Vec3f operator-(Vec3f a, Vec3f b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
        friend Vec3f operator*(Vec3f a, float s) { return {a.x * s, a.y * s, a.z * s}; }
        friend bool operator==(Vec3f a, Vec3f b) = default;
    };

    inline float dot(Vec3f a, Vec3f b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

    // Row-major: m[row][col]
    struct Mat3f
    {
        std::array<std::array<float, 3>, 3> m{};

        static Mat3f identity() { return {{{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}}}; }

        friend Vec3f operator*(const Mat3f &a, Vec3f v)
        {
            return {a.m[0][0] * v.x + a.m[0][1] * v.y + a.m[0][2] * v.z,
                    a.m[1][0] * v.x + a.m[1][1] * v.y + a.m[1][2] * v.z,
                    a.m[2][0] * v.x + a.m[2][1] * v.y + a.m[2][2] * v.z};
        }
        friend Mat3f operator*(const Mat3f &a, const Mat3f &b)
        {
            Mat3f r;
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    r.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j];
            return r;
        }

        float determinant() const
        {
            return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
                   m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                   m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
        }

        // Empty when the matrix is singular
        std::optional<Mat3f> inverse() const
        {
            float det = determinant();
            if (std::fabs(det) < 1e-12f)
                return std::nullopt;
            float inv = 1.0f / det;
            Mat3f r;
            r.m[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * inv;
            r.m[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * inv;
            r.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * inv;
            r.m[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * inv;
            r.m[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * inv;
            r.m[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * inv;
            r.m[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * inv;
            r.m[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * inv;
            r.m[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * inv;
            return r;
        }
    };
}

#endif
