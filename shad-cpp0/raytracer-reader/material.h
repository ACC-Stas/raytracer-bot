#pragma once

#include <vector.h>
#include <string>

struct Material {
    Material() {
        ambient_color = {0, 0, 0};
        diffuse_color = {0, 0, 0};
        specular_color = {0, 0, 0};
        intensity = {0, 0, 0};
        specular_exponent = 0;
        refraction_index = 0;
        albedo = {1, 0, 0};
    }
    Material& operator=(const Material& material) {
        name = material.name;
        ambient_color = material.ambient_color;
        diffuse_color = material.diffuse_color;
        specular_color = material.specular_color;
        intensity = material.intensity;
        specular_exponent = material.specular_exponent;
        refraction_index = material.refraction_index;
        albedo = material.albedo;
        return *this;
    }
    std::string name;
    Vector ambient_color;
    Vector diffuse_color;
    Vector specular_color;
    Vector intensity;
    double specular_exponent;
    double refraction_index;
    std::array<double, 3> albedo;
};
