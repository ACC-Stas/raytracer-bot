#include <catch.hpp>

#include <cmath>
#include <string>
#include <optional>

#include <camera_options.h>
#include <render_options.h>
#include <commons.hpp>
#include <raytracer.h>

int artifact_index = 0;
#ifdef SHAD_ARTIFACTS_DIR
const std::string kArtifactsDir = SHAD_ARTIFACTS_DIR;
#else
const std::string kArtifactsDir;
#endif

void CheckImage(const std::string& obj_filename, const std::string& result_filename,
                const CameraOptions& camera_options, const RenderOptions& render_options,
                std::optional<std::string> output_filename = std::nullopt) {
    auto image = Render(kBasePath + "tests/" + obj_filename, camera_options, render_options);
    if (output_filename.has_value()) {
        image.Write(output_filename.value());
    }

    if (!kArtifactsDir.empty()) {
        image.Write(kArtifactsDir + "/" + std::to_string(artifact_index++) + ".png");
    }

    Image ok_image(kBasePath + "tests/" + result_filename);
    Compare(image, ok_image);
}

TEST_CASE("Shading parts", "[raytracer]") {
    CameraOptions camera_opts(640, 480);
    RenderOptions render_opts{1};
    CheckImage("shading_parts/scene.obj", "shading_parts/scene.png", camera_opts, render_opts);
}

TEST_CASE("Triangle", "[raytracer]") {
    CameraOptions camera_opts(640, 480);
    camera_opts.look_from = std::array<double, 3>{0.0, 2.0, 0.0};
    camera_opts.look_to = std::array<double, 3>{0.0, 0.0, 0.0};
    RenderOptions render_opts{1};
    CheckImage("triangle/scene.obj", "triangle/scene.png", camera_opts, render_opts);
}

TEST_CASE("Triangle2", "[raytracer]") {
    CameraOptions camera_opts(640, 480);
    camera_opts.look_from = std::array<double, 3>{0.0, -2.0, 0.0};
    camera_opts.look_to = std::array<double, 3>{0.0, 0.0, 0.0};
    RenderOptions render_opts{1};
    CheckImage("triangle/scene.obj", "triangle/black.png", camera_opts, render_opts);
}

TEST_CASE("Classic box", "[raytracer]") {
    CameraOptions camera_opts(500, 500);
    camera_opts.look_from = std::array<double, 3>{-0.5, 1.5, 0.98};
    camera_opts.look_to = std::array<double, 3>{0.0, 1.0, 0.0};
    RenderOptions render_opts{4};
    CheckImage("classic_box/CornellBox-Original.obj", "classic_box/first.png", camera_opts,
               render_opts);
    camera_opts.look_from = std::array<double, 3>{-0.9, 1.9, -1};
    camera_opts.look_to = std::array<double, 3>{0.0, 0.0, 0};
    CheckImage("classic_box/CornellBox-Original.obj", "classic_box/second.png", camera_opts,
               render_opts);
}

TEST_CASE("Mirrors", "[raytracer]") {
    CameraOptions camera_opts(800, 600);
    camera_opts.look_from = std::array<double, 3>{2, 1.5, -0.1};
    camera_opts.look_to = std::array<double, 3>{1, 1.2, -2.8};
    RenderOptions render_opts{9};
    CheckImage("mirrors/scene.obj", "mirrors/result.png", camera_opts, render_opts);
}

TEST_CASE("Box with spheres", "[raytracer]") {
    CameraOptions camera_opts(640, 480, M_PI / 3);
    camera_opts.look_from = std::array<double, 3>{0.0, 0.7, 1.75};
    camera_opts.look_to = std::array<double, 3>{0.0, 0.7, 0.0};
    RenderOptions render_opts{4};
    CheckImage("box/cube.obj", "box/cube.png", camera_opts, render_opts);
}

TEST_CASE("Distorted box", "[raytracer]") {
    CameraOptions camera_opts(500, 500);
    camera_opts.look_from = std::array<double, 3>{-0.5, 1.5, 1.98};
    camera_opts.look_to = std::array<double, 3>{0.0, 1.0, 0.0};
    RenderOptions render_opts{4};
    CheckImage("distorted_box/CornellBox-Original.obj", "distorted_box/result.png", camera_opts,
               render_opts);
}

TEST_CASE("Deer", "[raytracer]") {
    CameraOptions camera_opts(500, 500);
    camera_opts.look_from = std::array<double, 3>{100, 200, 150};
    camera_opts.look_to = std::array<double, 3>{0.0, 100.0, 0.0};
    RenderOptions render_opts{1};
    CheckImage("deer/CERF_Free.obj", "deer/result.png", camera_opts, render_opts);
}
