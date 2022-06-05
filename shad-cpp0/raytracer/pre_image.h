#pragma once
#include <vector>
#include "vector.h"
#include "image.h"
#include <array>

struct PreImage {
    PreImage(size_t x_size, size_t y_size) {
        matrix = std::vector<std::vector<Vector>>(x_size);
        for (size_t i = 0; i < x_size; ++i) {
            matrix[i] = std::vector<Vector>(y_size);
        }
    }
    std::vector<std::vector<Vector>> matrix;
    void SetDefault(Vector def) {
        for (size_t x = 0; x < this->matrix.size(); ++x) {
            for (size_t y = 0; y < this->matrix[0].size(); ++y) {
                this->matrix[x][y] = def;
            }
        }
    };
};

Image MakeImage(const PreImage& pre_image) {
    Image result(pre_image.matrix.size(), pre_image.matrix[0].size());
    for (size_t x = 0; x < pre_image.matrix.size(); ++x) {
        for (size_t y = 0; y < pre_image.matrix[0].size(); ++y) {
            const Vector& vector = pre_image.matrix[x][y];
            result.SetPixel({static_cast<int>(vector[0] * 255), static_cast<int>(vector[1] * 255),
                             static_cast<int>(vector[2] * 255)},
                            y, x);
        }
    }
    return result;
}
