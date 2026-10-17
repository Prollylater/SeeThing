#ifndef CORE_GEOMETRY_H
#define CORE_GEOMETRY_H

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>

// Small fixed-size math: colour matrices, 3x3 kernels, 2D transforms. Image-sized data lives in Image<T>.
namespace core
{
    struct Vec2f
    {
        float x = 0, y = 0;

        friend Vec2f operator+(Vec2f a, Vec2f b) { return {a.x + b.x, a.y + b.y}; }
        friend Vec2f operator-(Vec2f a, Vec2f b) { return {a.x - b.x, a.y - b.y}; }
        friend Vec2f operator*(Vec2f a, float s) { return {a.x * s, a.y * s}; }
        friend Vec2f operator/(Vec2f a, float s) { return {a.x / s, a.y / s}; }
        Vec2f &operator+=(Vec2f b) { return *this = *this + b; }
        friend bool operator==(Vec2f a, Vec2f b) = default;
    };

    struct Vec2i
    {
        int x = 0, y = 0;
        friend bool operator==(Vec2i a, Vec2i b) = default;
    };

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

    // Pixel rectangle: x, y is the top-left pixel
    struct Recti
    {
        int x = 0, y = 0, w = 0, h = 0;

        bool empty() const { return w <= 0 || h <= 0; }
        int right() const { return x + w; }
        int bottom() const { return y + h; }
        friend bool operator==(Recti a, Recti b) = default;
    };

    inline Recti unite(Recti a, Recti b)
    {
        if (a.empty())
            return b;
        if (b.empty())
            return a;
        const int x = std::min(a.x, b.x), y = std::min(a.y, b.y);
        return {x, y, std::max(a.right(), b.right()) - x, std::max(a.bottom(), b.bottom()) - y};
    }

    // Empty when they don't overlap
    inline Recti intersect(Recti a, Recti b)
    {
        const int x = std::max(a.x, b.x), y = std::max(a.y, b.y);
        const int r = std::min(a.right(), b.right()), bt = std::min(a.bottom(), b.bottom());
        return r > x && bt > y ? Recti{x, y, r - x, bt - y} : Recti{};
    }

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
