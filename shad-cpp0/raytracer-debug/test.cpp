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
    RenderOptions render_opts{1, RenderMode::kDepth};
    CheckImage("shading_parts/scene.obj", "shading_parts/depth.png", camera_opts, render_opts);
    render_opts.mode = RenderMode::kNormal;
    CheckImage("shading_parts/scene.obj", "shading_parts/normal.png", camera_opts, render_opts);
}

TEST_CASE("Triangle", "[raytracer]") {
    CameraOptions camera_opts(640, 480);
    camera_opts.look_from = std::array<double, 3>{0.0, 2.0, 0.0};
    camera_opts.look_to = std::array<double, 3>{0.0, 0.0, 0.0};
    RenderOptions render_opts{1, RenderMode::kDepth};
    CheckImage("triangle/scene.obj", "triangle/depth.png", camera_opts, render_opts);
    render_opts.mode = RenderMode::kNormal;
    CheckImage("triangle/scene.obj", "triangle/normal.png", camera_opts, render_opts);
}

TEST_CASE("Triangle2", "[raytracer]") {
    CameraOptions camera_opts(640, 480);
    camera_opts.look_from = std::array<double, 3>{0.0, -2.0, 0.0};
    camera_opts.look_to = std::array<double, 3>{0.0, 0.0, 0.0};
    RenderOptions render_opts{1, RenderMode::kDepth};
    CheckImage("triangle/scene.obj", "triangle/depth2.png", camera_opts, render_opts);
    render_opts.mode = RenderMode::kNormal;
    CheckImage("triangle/scene.obj", "triangle/normal2.png", camera_opts, render_opts);
}

TEST_CASE("Classic box", "[raytracer]") {
    CameraOptions camera_opts(500, 500);
    camera_opts.look_from = std::array<double, 3>{-0.5, 1.5, 0.98};
    camera_opts.look_to = std::array<double, 3>{0.0, 1.0, 0.0};
    RenderOptions render_opts{4, RenderMode::kDepth};
    CheckImage("classic_box/CornellBox-Original.obj", "classic_box/depth1.png", camera_opts,
               render_opts);
    render_opts.mode = RenderMode::kNormal;
    CheckImage("classic_box/CornellBox-Original.obj", "classic_box/normal1.png", camera_opts,
               render_opts);
    camera_opts.look_from = std::array<double, 3>{-0.9, 1.9, -1};
    camera_opts.look_to = std::array<double, 3>{0.0, 0.0, 0};
    render_opts.mode = RenderMode::kDepth;
    CheckImage("classic_box/CornellBox-Original.obj", "classic_box/depth2.png", camera_opts,
               render_opts);
    render_opts.mode = RenderMode::kNormal;
    CheckImage("classic_box/CornellBox-Original.obj", "classic_box/normal2.png", camera_opts,
               render_opts);
}

TEST_CASE("Box with spheres", "[raytracer]") {
    CameraOptions camera_opts(640, 480, M_PI / 3);
    camera_opts.look_from = std::array<double, 3>{0.0, 0.7, 1.75};
    camera_opts.look_to = std::array<double, 3>{0.0, 0.7, 0.0};
    RenderOptions render_opts{4, RenderMode::kDepth};
    CheckImage("box/cube.obj", "box/depth.png", camera_opts, render_opts);
    render_opts.mode = RenderMode::kNormal;
    CheckImage("box/cube.obj", "box/normal.png", camera_opts, render_opts);
}

TEST_CASE("Deer", "[raytracer]") {
    CameraOptions camera_opts(500, 500);
    camera_opts.look_from = std::array<double, 3>{100, 200, 150};
    camera_opts.look_to = std::array<double, 3>{0.0, 100.0, 0.0};
    RenderOptions render_opts{1, RenderMode::kDepth};
    CheckImage("deer/CERF_Free.obj", "deer/depth.png", camera_opts, render_opts);
    render_opts.mode = RenderMode::kNormal;
    CheckImage("deer/CERF_Free.obj", "deer/normal.png", camera_opts, render_opts);
}
