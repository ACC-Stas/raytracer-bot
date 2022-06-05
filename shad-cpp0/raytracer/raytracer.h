#pragma once

#include "image.h"
#include "camera_options.h"
#include "render_options.h"
#include <string>
#include "vector.h"
#include "ray.h"
#include "scene.h"
#include "geometry.h"
#include "pre_image.h"
#include <vector>

Vector CamToWorld(const Vector& t, const Vector& right, const Vector& up, const Vector& forward,
                  const Vector& origin) {
    Vector result = Vector();
    result[0] = right[0] * origin[0] + up[0] * origin[1] + forward[0] * origin[2] + t[0];
    result[1] = right[1] * origin[0] + up[1] * origin[1] + forward[1] * origin[2] + t[1];
    result[2] = right[2] * origin[0] + up[2] * origin[1] + forward[2] * origin[2] + t[2];
    return result;
}

class RayGetter {
public:
    RayGetter(const CameraOptions& camera_options) {
        forward_ = Vector(camera_options.look_from) - Vector(camera_options.look_to);
        forward_.Normalize();
        if (forward_ == Vector{0, 1, 0}) {
            right_ = {-1, 0, 0};
            up_ = {0, 0, -1};
        } else if (forward_ == Vector{0, -1, 0}) {
            right_ = {1, 0, 0};
            up_ = {0, 0, 1};
        } else {
            Vector tmp = {0, 1, 0};
            right_ = CrossProduct(tmp, forward_);
            right_.Normalize();
            up_ = CrossProduct(forward_, right_);
            up_.Normalize();
        }
    }

    Ray operator()(const CameraOptions& camera_options, int x, int y) {
        double dir_x = (2. * (x + 0.5) / static_cast<double>(camera_options.screen_width) - 1) *
                       tan(camera_options.fov / 2.) * camera_options.screen_width /
                       static_cast<double>(camera_options.screen_height),
               dir_y = -(2. * (y + 0.5) / static_cast<double>(camera_options.screen_height) - 1) *
                       tan(camera_options.fov / 2.),
               dir_z = -1.;

        Vector ordinary_direction = {dir_x, dir_y, dir_z};
        ordinary_direction.Normalize();

        return Ray(CamToWorld(camera_options.look_from, right_, up_, forward_, {0, 0, 0}),
                   CamToWorld(camera_options.look_from, right_, up_, forward_, ordinary_direction) -
                       CamToWorld(camera_options.look_from, right_, up_, forward_, {0, 0, 0}));
    }

private:
    Vector forward_;
    Vector right_;
    Vector up_;
};

Image GetDepthImage(const std::string& filename, const CameraOptions& camera_options) {
    Image result(camera_options.screen_width, camera_options.screen_height);
    Scene scene = ReadScene(filename);
    double max_d = 0;
    double d;

    RayGetter get_ray(camera_options);

    for (int x = 0; x < camera_options.screen_width; ++x) {
        for (int y = 0; y < camera_options.screen_height; ++y) {
            Ray ray = get_ray(camera_options, x, y);
            double current_min = static_cast<double>(INT64_MAX);
            bool have_intersection = false;
            for (const Object& object : scene.GetObjects()) {
                std::optional<Intersection> intersection = GetIntersection(ray, object.polygon);
                if (intersection.has_value()) {
                    d = intersection->GetDistance();
                    have_intersection = true;
                    if (d < current_min) {
                        current_min = d;
                    }
                }
            }
            for (const SphereObject& object : scene.GetSphereObjects()) {
                std::optional<Intersection> intersection = GetIntersection(ray, object.sphere);
                if (intersection.has_value()) {
                    d = intersection->GetDistance();
                    have_intersection = true;
                    if (d < current_min) {
                        current_min = d;
                    }
                }
            }
            if (have_intersection && current_min > max_d) {
                max_d = current_min;
            }
        }
    }

    for (int x = 0; x < camera_options.screen_width; ++x) {
        for (int y = 0; y < camera_options.screen_height; ++y) {
            result.SetPixel({255, 255, 255}, y, x);
        }
    }

    for (int x = 0; x < camera_options.screen_width; ++x) {
        for (int y = 0; y < camera_options.screen_height; ++y) {
            Ray ray = get_ray(camera_options, x, y);
            bool have_intersection = false;
            for (const Object& object : scene.GetObjects()) {
                std::optional<Intersection> intersection = GetIntersection(ray, object.polygon);
                if (intersection.has_value()) {
                    have_intersection = true;
                    d = intersection->GetDistance();
                    if (255 * (d / max_d) > result.GetPixel(y, x).r) {
                        continue;
                    }
                    result.SetPixel(
                        {static_cast<int>(255 * (d / max_d)), static_cast<int>(255 * (d / max_d)),
                         static_cast<int>(255 * (d / max_d))},
                        y, x);
                }
            }
            for (const SphereObject& object : scene.GetSphereObjects()) {
                std::optional<Intersection> intersection = GetIntersection(ray, object.sphere);
                if (intersection.has_value()) {
                    have_intersection = true;
                    d = intersection->GetDistance();
                    if (255 * (d / max_d) > result.GetPixel(y, x).r) {
                        continue;
                    }
                    result.SetPixel(
                        {static_cast<int>(255 * (d / max_d)), static_cast<int>(255 * (d / max_d)),
                         static_cast<int>(255 * (d / max_d))},
                        y, x);
                }
            }

            if (!have_intersection) {
                result.SetPixel({255, 255, 255}, y, x);
            }
        }
    }
    return result;
}

RGB VectorToRGB(const Vector& vector) {
    RGB result;
    result.r = static_cast<int>(255 * vector[0]);
    result.g = static_cast<int>(255 * vector[1]);
    result.b = static_cast<int>(255 * vector[2]);
    return result;
}

Vector GetObjectNormal(const Object& object, const Intersection& intersection) {
    Vector vector;
    if (object.have_normal) {
        Vector barycentric_coord = GetBarycentricCoords(object.polygon, intersection.GetPosition());
        vector = object.normal_triangle.GetVertex(0) * barycentric_coord[0] +
                 object.normal_triangle.GetVertex(1) * barycentric_coord[1] +
                 object.normal_triangle.GetVertex(2) * barycentric_coord[2];

    } else {
        vector = intersection.GetNormal();
    }
    return vector;
}

Image GetNormalImage(const std::string& filename, const CameraOptions& camera_options) {
    Image result(camera_options.screen_width, camera_options.screen_height);
    PreImage distance_image(camera_options.screen_width, camera_options.screen_height);
    distance_image.SetDefault({static_cast<double>(INT64_MAX), static_cast<double>(INT64_MAX),
                               static_cast<double>(INT64_MAX)});
    Scene scene = ReadScene(filename);
    RayGetter get_ray(camera_options);

    for (int x = 0; x < camera_options.screen_width; ++x) {
        for (int y = 0; y < camera_options.screen_height; ++y) {
            Ray ray = get_ray(camera_options, x, y);
            bool have_intersection = false;

            for (const Object& object : scene.GetObjects()) {
                std::optional<Intersection> intersection = GetIntersection(ray, object.polygon);
                if (intersection.has_value()) {
                    have_intersection = true;
                    if (intersection->GetDistance() > distance_image.matrix[x][y][0]) {
                        continue;
                    }
                    distance_image.matrix[x][y] = {intersection->GetDistance(),
                                                   intersection->GetDistance(),
                                                   intersection->GetDistance()};
                    Vector vector = GetObjectNormal(object, *intersection);
                    vector *= 0.5;
                    vector[0] += 0.5;
                    vector[1] += 0.5;
                    vector[2] += 0.5;
                    result.SetPixel({VectorToRGB(vector)}, y, x);
                }
            }

            for (const SphereObject& object : scene.GetSphereObjects()) {
                std::optional<Intersection> intersection = GetIntersection(ray, object.sphere);
                if (intersection.has_value()) {
                    have_intersection = true;
                    if (intersection->GetDistance() > distance_image.matrix[x][y][0]) {
                        continue;
                    }
                    distance_image.matrix[x][y] = {intersection->GetDistance(),
                                                   intersection->GetDistance(),
                                                   intersection->GetDistance()};
                    Vector vector = intersection->GetNormal();
                    vector *= 0.5;
                    vector[0] += 0.5;
                    vector[1] += 0.5;
                    vector[2] += 0.5;
                    result.SetPixel({VectorToRGB(vector)}, y, x);
                }
            }

            if (!have_intersection) {
                result.SetPixel({0, 0, 0}, y, x);
            }
        }
    }
    return result;
}

Vector PostProcessing(const Vector& vector, double max) {
    Vector result;
    result[0] = vector[0] * (1 + vector[0] / (max * max)) / (1 + vector[0]);
    result[1] = vector[1] * (1 + vector[1] / (max * max)) / (1 + vector[1]);
    result[2] = vector[2] * (1 + vector[2] / (max * max)) / (1 + vector[2]);

    double gamma_power = 1.0 / 2.2;

    result[0] = std::pow(result[0], gamma_power);
    result[1] = std::pow(result[1], gamma_power);
    result[2] = std::pow(result[2], gamma_power);

    return result;
}

std::tuple<Material, std::optional<Intersection>> GetSceneIntersection(const Ray& ray,
                                                                       const Scene& scene) {
    double min_dist = static_cast<double>(INT64_MAX);
    Material material;
    std::optional<Intersection> close_intersection;

    for (const Object& object : scene.GetObjects()) {
        std::optional<Intersection> intersection = GetIntersection(ray, object.polygon);
        if (!intersection.has_value()) {
            continue;
        }
        double d = intersection->GetDistance();
        if (d > min_dist) {
            continue;
        }
        min_dist = d;
        material = *object.material;
        close_intersection = intersection;
        close_intersection->SetNormal(GetObjectNormal(object, *intersection));
    }

    for (const SphereObject& object : scene.GetSphereObjects()) {
        std::optional<Intersection> intersection = GetIntersection(ray, object.sphere);
        if (!intersection.has_value()) {
            continue;
        }
        double d = intersection->GetDistance();
        if (d > min_dist) {
            continue;
        }
        material = *object.material;
        close_intersection = intersection;
        min_dist = d;
    }
    return std::make_tuple(material, close_intersection);
}

bool ReachLight(const Scene& scene, const Light& light,
                const std::optional<Intersection>& near_intersection) {
    Vector direction = light.position - near_intersection->GetPosition();
    direction.Normalize();
    Ray ray = Ray(near_intersection->GetPosition(), direction);
    double required_dist = Length(light.position - near_intersection->GetPosition());

    for (const Object& object : scene.GetObjects()) {
        std::optional<Intersection> intersection = GetIntersection(ray, object.polygon);
        if (!intersection.has_value()) {
            continue;
        }
        if (intersection->GetDistance() < required_dist) {
            return false;
        }
    }
    for (const SphereObject& object : scene.GetSphereObjects()) {
        std::optional<Intersection> intersection = GetIntersection(ray, object.sphere);
        if (!intersection.has_value()) {
            continue;
        }
        if (intersection->GetDistance() < required_dist) {
            return false;
        }
    }
    return true;
}

Vector GetLight(const Scene& scene, const Ray& ray, const RenderOptions& render_options,
                const int depth, bool need_refract);

Vector GetRefractLight(const Ray& ray, const Intersection& near_intersection,
                       const Material& material, const Scene& scene,
                       const RenderOptions& render_options, int depth, bool need_refract) {
    const double epsilon = -1e-8;
    Vector refract;
    if (material.albedo[2] == 0) {
        return refract;
    }
    std::optional<Vector> direction;
    if (need_refract) {
        direction = Refract(ray.GetDirection(), near_intersection.GetNormal(),
                            1. / material.refraction_index);
    } else {
        direction =
            Refract(ray.GetDirection(), near_intersection.GetNormal(), material.refraction_index);
    }
    if (direction.has_value()) {
        Vector position = near_intersection.GetPosition();
        position += epsilon * near_intersection.GetNormal();
        Ray refract_ray = Ray(position, direction.value());
        if (need_refract) {
            refract += GetLight(scene, refract_ray, render_options, depth + 1, false);
        } else {
            refract +=
                material.albedo[2] * GetLight(scene, refract_ray, render_options, depth + 1, true);
        }
    }
    return refract;
}

Vector GetLight(const Scene& scene, const Ray& ray, const RenderOptions& render_options,
                const int depth, bool need_refract) {
    const double epsilon = -1e-8;
    auto [material, near_intersection] = GetSceneIntersection(ray, scene);

    if (depth > render_options.depth || !near_intersection.has_value()) {
        return Vector({0.0, 0.0, 0.0});
    }

    Vector reflect = Vector({0, 0, 0});
    if (material.albedo[1] != 0 && !need_refract) {
        Vector reflect_dir = Reflect(ray.GetDirection(), near_intersection->GetNormal());
        Vector position = near_intersection->GetPosition();
        position += +epsilon * near_intersection->GetNormal();
        reflect = GetLight(scene, Ray(position, reflect_dir), render_options, depth + 1, false);
    }

    Vector refract = GetRefractLight(ray, near_intersection.value_or(Intersection()), material,
                                     scene, render_options, depth, need_refract);

    Vector diffuse_light;
    Vector specular_light;
    for (const Light& light : scene.GetLights()) {
        Vector light_dir = light.position - near_intersection->GetPosition();
        light_dir.Normalize();

        if (!ReachLight(scene, light, near_intersection)) {
            continue;
        }

        diffuse_light += light.intensity * DotProduct(light_dir, near_intersection->GetNormal());
        Vector reflection = Reflect(-1 * light_dir, near_intersection->GetNormal());
        double angel = std::max(0., DotProduct(-1 * reflection, ray.GetDirection()));
        specular_light += std::pow(angel, material.specular_exponent) * light.intensity;
    }

    Vector light;
    light += material.diffuse_color * diffuse_light * material.albedo[0];
    light += material.specular_color * specular_light * material.albedo[0];
    light += reflect * material.albedo[1] + refract + material.ambient_color;
    light += material.intensity;
    return light;
}

Image GetFullImage(const std::string& filename, const CameraOptions& camera_options,
                   const RenderOptions& render_options) {
    Image result(camera_options.screen_width, camera_options.screen_height);
    Scene scene = ReadScene(filename);
    PreImage pre_image(camera_options.screen_width, camera_options.screen_height);
    RayGetter get_ray(camera_options);
    double max_light = 0;
    const bool need_refract = false;

    for (int x = 0; x < camera_options.screen_width; ++x) {
        for (int y = 0; y < camera_options.screen_height; ++y) {
            Ray ray = get_ray(camera_options, x, y);
            pre_image.matrix[x][y] = GetLight(scene, ray, render_options, 1, need_refract);
            max_light = std::max({pre_image.matrix[x][y][0], pre_image.matrix[x][y][1],
                                  pre_image.matrix[x][y][2], max_light});
        }
    }
    for (int x = 0; x < camera_options.screen_width; ++x) {
        for (int y = 0; y < camera_options.screen_height; ++y) {
            Vector rgb_v = PostProcessing(pre_image.matrix[x][y], max_light);
            result.SetPixel(VectorToRGB(rgb_v), y, x);
        }
    }
    return result;
}

Image Render(const std::string& filename, const CameraOptions& camera_options,
             const RenderOptions& render_options) {
    if (render_options.mode == RenderMode::kDepth) {
        return GetDepthImage(filename, camera_options);
    }
    if (render_options.mode == RenderMode::kNormal) {
        return GetNormalImage(filename, camera_options);
    }
    if (render_options.mode == RenderMode::kFull) {
        return GetFullImage(filename, camera_options, render_options);
    }

    return Image(1, 1);
}
