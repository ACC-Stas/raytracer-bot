#pragma once

#include <triangle.h>
#include <material.h>
#include <sphere.h>
#include <array>
#include <optional>

struct Object {
    Object(const Material *material) : material(material) {
    }

    const Material *material = nullptr;
    Triangle polygon = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    Triangle normal_triangle = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    bool have_normal = false;

    const Vector *GetNormal(size_t index) const {
        return &normal_triangle.GetVertex(index);
    }
};

struct SphereObject {
    SphereObject(const Material *material) : material(material) {
    }
    SphereObject(const Material *material, const Sphere &sphere) : SphereObject(material) {
        this->sphere = sphere;
    }

    const Material *material = nullptr;
    Sphere sphere = Sphere({0, 0, 0}, 0);
};
