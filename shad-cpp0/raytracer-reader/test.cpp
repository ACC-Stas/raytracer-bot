#include <catch.hpp>

#include <scene.h>

#ifndef SHAD_TASK_DIR
#define SHAD_TASK_DIR "./"
#endif

TEST_CASE("Scene", "[raytracer]") {
    const std::string dir_path(SHAD_TASK_DIR);
    const auto scene = ReadScene(dir_path + "tests/box/cube.obj");
    const double eps = 1e-6;

    const auto& materials_map = scene.GetMaterials();
    REQUIRE(materials_map.size() == 9);

    // objects
    const auto& objects = scene.GetObjects();
    REQUIRE(objects.size() == 10);

    const Vector& vertex_coord_check = objects[0].polygon.GetVertex(0);
    REQUIRE(std::fabs(vertex_coord_check[0] - 1.) < eps);
    REQUIRE(std::fabs(vertex_coord_check[1] - 0.) < eps);
    REQUIRE(std::fabs(vertex_coord_check[2] - (-1.04)) < eps);

    const Vector& normal_check = *objects[1].GetNormal(1);
    REQUIRE(std::fabs(normal_check[0] - 0.) < eps);
    REQUIRE(std::fabs(normal_check[1] - 1.) < eps);
    REQUIRE(std::fabs(normal_check[2] - 0.) < eps);

    for (const auto& object : objects) {
        REQUIRE(materials_map.find(object.material->name) != materials_map.end());
    }

    // spheres
    const auto& spheres = scene.GetSphereObjects();
    REQUIRE(spheres.size() == 2);

    const Vector& center = spheres[0].sphere.GetCenter();
    REQUIRE(std::fabs(center[0] - (-0.4)) < eps);
    REQUIRE(std::fabs(center[1] - (0.3)) < eps);
    REQUIRE(std::fabs(center[2] - (-0.4)) < eps);
    REQUIRE(std::fabs(spheres[0].sphere.GetRadius() - 0.3) < eps);
    for (const auto& sphere : spheres) {
        REQUIRE(materials_map.find(sphere.material->name) != materials_map.end());
    }

    // lights
    const auto& lights = scene.GetLights();
    REQUIRE(lights.size() == 2);
    REQUIRE(std::fabs(lights[0].position[1] - 1.5899) < eps);
    REQUIRE(std::fabs(lights[0].intensity[0] - 1.) < eps);
    REQUIRE(std::fabs(lights[1].position[2] - 1.98) < eps);
    REQUIRE(std::fabs(lights[1].intensity[1] - 0.5) < eps);
    REQUIRE(std::fabs(lights[1].intensity[2] - 0.5) < eps);

    // materials
    const Material& right_sphere = materials_map.at("rightSphere");
    REQUIRE(std::fabs(right_sphere.albedo[0] - 0.) < eps);
    REQUIRE(std::fabs(right_sphere.albedo[1] - 0.3) < eps);
    REQUIRE(std::fabs(right_sphere.albedo[2] - 0.7) < eps);
    REQUIRE(std::fabs(right_sphere.specular_exponent - 1024) < eps);
    REQUIRE(std::fabs(right_sphere.refraction_index - 1.8) < eps);

    const Material& light = materials_map.at("light");
    REQUIRE(std::fabs(light.ambient_color[1] - 0.78) < eps);
    REQUIRE(std::fabs(light.diffuse_color[2] - 0.78) < eps);
    REQUIRE(std::fabs(light.specular_color[1] - 0.) < eps);
    REQUIRE(std::fabs(light.intensity[2] - 1.) < eps);

    const Vector& wall_behind_diffuse = materials_map.at("wallBehind").diffuse_color;
    REQUIRE(std::fabs(wall_behind_diffuse[0] - 0.2) < eps);
    REQUIRE(std::fabs(wall_behind_diffuse[1] - 0.7) < eps);
    REQUIRE(std::fabs(wall_behind_diffuse[2] - 0.8) < eps);
}