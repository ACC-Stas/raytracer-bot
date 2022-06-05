#pragma once

#include <vector.h>

class Intersection {
public:
    Intersection(Vector pos, Vector norm, double dist)
        : position_(pos), normal_(norm), distance_(dist) {
    }

    Intersection() : position_({0, 0, 0}), normal_({0, 0, 0}), distance_(0) {
    }

    Intersection& operator=(const Intersection& intersection) {
        position_ = intersection.position_;
        normal_ = intersection.normal_;
        distance_ = intersection.distance_;
        return *this;
    }

    const Vector& GetPosition() const {
        return position_;
    }
    const Vector& GetNormal() const {
        return normal_;
    }

    void SetNormal(const Vector& normal) {
        normal_ = normal;
    }

    double GetDistance() const {
        return distance_;
    }

private:
    Vector position_;
    Vector normal_;
    double distance_;
};
