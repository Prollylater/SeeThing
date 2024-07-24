#ifndef VEC_H
#define VEC_H
#include <iostream>
#include <cstring>
#include <stdexcept>

template <typename T>
struct Vec
{
  T x, y, z;

  Vec() : x(0), y(0), z(0) {}
  Vec(T _x, T _y, T _z) : x(_x), y(_y), z(_z){};
  Vec(const Vec &other) : x(other.x), y(other.y), z(other.z) {}
  Vec &operator=(const Vec &other)
  {
    if (this != &other)
    {
      x = other.x;
      y = other.y;
      z = other.z;
    }
    return *this;
  }

  template <typename T2>
  Vec<T2> convertDt() const
  {
    static_assert(std::is_convertible<T, T2>::value,
                  "Type used in convertDt not convertible to target type");
    T2 t2x, t2y, t2z;

    t2x = static_cast<T2>(x);
    t2y = static_cast<T2>(y);
    t2z = static_cast<T2>(z);

    Vec<T2> newvec(t2x, t2y, t2z);
    return newvec;
  };

  // Destructor
  ~Vec() {}

  // Move constructor
  Vec(Vec &&other) noexcept : x(std::move(other.x)), y(std::move(other.y)), z(std::move(other.z))
  {
    other = Vec();
  }

  T sumComponents()
  {
    return x + y + z;
  }
  // Move assignment operator
  Vec &operator=(Vec &&other) noexcept
  {
    if (this != &other)
    {
      x = std::move(other.x);
      y = std::move(other.y);
      z = std::move(other.z);
      other = Vec();
    }
    return *this;
  }
  // ConstructorS

  // Vector Operations

  // Vector Addition
  Vec<T> operator+(const Vec<T> &v2)
  {
    T tmpx = (x + v2.x);
    T tmpxy = (y + v2.y);
    T tmpxz = (z + v2.z);
    return Vec<T>(tmpx, tmpxy, tmpxz);
  }

  Vec<T> &operator+=(const Vec<T> &otherMat);

  // Vector Subtraction
  Vec<T> operator-(const Vec<T> &v2)
  {
    T tmpx = (x - v2.x);
    T tmpxy = (y - v2.y);
    T tmpxz = (z - v2.z);
    return Vec<T>(tmpx, tmpxy, tmpxz);
  }

  Vec<T> &operator-=(const Vec<T> &otherMat);
  // Division
  Vec<T> &operator/=(const T &scalar);

  //  Scalar Multiplication
  Vec<T> operator*(int a) const
  {
    T tmpx = (x * a);
    T tmpxy = (y * a);
    T tmpxz = (z * a);
    return Vec<T>(tmpx, tmpxy, tmpxz);
  }
  Vec<T> operator*(float a) const
  {
    T tmpx = (x * a);
    T tmpxy = (y * a);
    T tmpxz = (z * a);
    return Vec<T>(tmpx, tmpxy, tmpxz);
  }
  Vec<T> &operator*=(const T &scalar);

  //  Scalar Division

  Vec<T> operator/(int a) const
  {
    T tmpx = (x / a);
    T tmpxy = (y / a);
    T tmpxz = (z / a);
    return Vec<T>(tmpx, tmpxy, tmpxz);
  }
  Vec<T> operator/(float a) const
  {
    T tmpx = (x / a);
    T tmpxy = (y / a);
    T tmpxz = (z / a);
    return Vec<T>(tmpx, tmpxy, tmpxz);
  }

  // Comparison

  bool operator<(const Vec<T> vec) const
  {
    return (x < vec.x) || ((x == vec.x) && ((y < vec.y) || ((y == vec.y) && (z < vec.z))));
  }

  
  bool operator!=(const Vec<T> vec) const
  {
    return (x != vec.x) && (y != vec.y) && (z != vec.z);
  }

  friend std::ostream &operator<<(std::ostream &os, const Vec<T> &vec)
  {
    os << "x: " << vec.x
       << "y: " << vec.y
       << "z: " << vec.z << std::endl;
    return os;
  }
};

#include <cmath>
#include <algorithm>

// Vector Operations
// Dot Product
template <typename T>
float Dot(const Vec<T> &v1, const Vec<T> &v2)
{
  // Example operation with the templated class instance
  return v1.x * v2.x + (v1.y * v2.y) + v1.z * v2.z;
}

// Cross Product
template <typename T>
Vec<T> Cross(const Vec<T> &v1, const Vec<T> &v2)
{
  // Example operation with the templated class instance
  T x = (v1.y * v2.z) - (v1.z * v2.y);
  T y = (v1.z * v2.x) - (v1.x * v2.z);
  T z = (v1.x * v2.y) - (v1.y * v2.x);
  return Vec<T>(x, y, z);
}

// Magnitude
template <typename T>
float Magn(const Vec<T> &vector)
{
  return std::sqrt((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
}

// Projection
template <typename T>
float ScalProj(const Vec<T> &v1, const Vec<T> &v2)
{
  return Dot(v1, v2) / Magn(v2);
}

template <typename T>
Vec<T> VecProj(const Vec<T> &v1, const Vec<T> &v2)
{

  return v2 * (Dot<T>(v1, v2) / Dot<T>(v2, v2));
}

// Normalization
template <typename T>
Vec<T> Norm(const Vec<T> &vec)
{
  float magn = Magn(vec);
  return Vec<T>(vec.x * (1.0f / magn), vec.y * (1.0f / magn), vec.z * (1.0f / magn));
}

//
//  OPERATORS
//
template <typename T>
Vec<T> &Vec<T>::operator*=(const T &scalar)
{
  x /= scalar;
  y /= scalar;
  z /= scalar;
  return *this;
}
// Addition
template <typename T>
Vec<T> &Vec<T>::operator+=(const Vec<T> &v2)
{
  x += v2.x;
  y += v2.y;
  z += v2.z;
  return *this;
}
// Soustraction
template <typename T>
Vec<T> &Vec<T>::operator-=(const Vec<T> &v2)
{
  x -= v2.x;
  y -= v2.y;
  z -= v2.z;
  return *this;
}

// Division
template <typename T>
Vec<T> &Vec<T>::operator/=(const T &scalar)
{
  x /= scalar;
  y /= scalar;
  z /= scalar;
  return *this;
}
#endif
