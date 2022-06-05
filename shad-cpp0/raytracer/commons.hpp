#pragma once

#include <image.h>

#include <cmath>
#include <string>
#include <optional>

#ifndef SHAD_TASK_DIR
#define SHAD_TASK_DIR "."
#endif

inline std::string ConstructBasePath() {
    std::string result(SHAD_TASK_DIR);
    return result;
}

const std::string kBasePath = ConstructBasePath();

inline double PixelDistance(const RGB& lhs, const RGB& rhs) {
    return sqrt(std::pow(lhs.r - rhs.r, 2.0) + std::pow(lhs.g - rhs.g, 2.0) +
                std::pow(lhs.b - rhs.b, 2.0));
}

inline void Compare(const Image& actual, const Image& expected) {
    static const double kEps = 2;
    int matches = 0;

    REQUIRE(actual.Width() == expected.Width());
    REQUIRE(actual.Height() == expected.Height());
    for (int y = 0; y < actual.Height(); ++y) {
        for (int x = 0; x < actual.Width(); ++x) {
            auto actual_data = actual.GetPixel(y, x);
            auto expected_data = expected.GetPixel(y, x);
            auto diff = PixelDistance(actual_data, expected_data);
            matches += diff < kEps;
        }
    }

    double similarity = static_cast<double>(matches) / (actual.Width() * actual.Height());
    REQUIRE(similarity >= 0.99);
}