#pragma once

#include <vector.h>

class Triangle {
public:
    Triangle(std::initializer_list<Vector> list) {
        size_t idx = 0;
        for (const Vector& v : list) {
            vertices_[idx] = v;
            ++idx;
        }
    }
    double Area() const {
        Vector cross = CrossProduct(vertices_[0] - vertices_[1], vertices_[0] - vertices_[2]);
        return Length(cross) / 2;
    }

    const Vector& GetVertex(size_t ind) const {
        return vertices_[ind];
    }

private:
    std::array<Vector, 3> vertices_;
};
