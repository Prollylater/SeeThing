#include <iostream>
#include <cmath>
#include "Vec.h" // Assuming Vec class is defined in Vec.h

// Helper function to compare floating point numbers
bool floatEquals(float a, float b, float tolerance = 1e-5) {
    return std::abs(a - b) <= tolerance;
}

int main() {
    // Create vectors for testing
    Vec<float> v1(1.0f, 2.0f, 3.0f);
    Vec<float> v2(4.0f, 5.0f, 6.0f);

    // Test vector addition
    Vec<float> vAdd = v1 + v2;
    std::cout << "Vector Addition: (" << vAdd.x << ", " << vAdd.y << ", " << vAdd.z << ")" << std::endl;

    // Test vector subtraction
    Vec<float> vSub = v1 - v2;
    std::cout << "Vector Subtraction: (" << vSub.x << ", " << vSub.y << ", " << vSub.z << ")" << std::endl;

    // Test scalar multiplication
    Vec<float> vScalarMul = v1 * 2.0f;
    std::cout << "Scalar Multiplication: (" << vScalarMul.x << ", " << vScalarMul.y << ", " << vScalarMul.z << ")" << std::endl;

    // Test dot product
    float dot = Dot(v1, v2);
    std::cout << "Dot Product: " << dot << std::endl;

    // Test cross product
    Vec<float> cross = Cross(v1, v2);
    std::cout << "Cross Product: (" << cross.x << ", " << cross.y << ", " << cross.z << ")" << std::endl;

    // Test magnitude
    float magnitude = Magn(v1);
    std::cout << "Magnitude of v1: " << magnitude << std::endl;

    // Test scalar projection
    float scalarProjection = ScalProj(v1, v2);
    std::cout << "Scalar Projection of v1 onto v2: " << scalarProjection << std::endl;

    // Test vector projection
    Vec<float> vectorProjection = VecProj(v1, v2);
    std::cout << "Vector Projection of v1 onto v2: (" << vectorProjection.x << ", " << vectorProjection.y << ", " << vectorProjection.z << ")" << std::endl;

    // Test normalization
    Vec<float> normalized = Norm(v1);
    std::cout << "Normalized v1: (" << normalized.x << ", " << normalized.y << ", " << normalized.z << ")" << std::endl;

    // Check normalization result
    float normMagnitude = Magn(normalized);
    if (floatEquals(normMagnitude, 1.0f)) {
        std::cout << "Normalization Test Passed: Magnitude is approximately 1" << std::endl;
    } else {
        std::cout << "Normalization Test Failed: Magnitude is not 1" << std::endl;
    }

    return 0;
}
