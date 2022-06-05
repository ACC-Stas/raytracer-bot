add_catch(test_raytracer_reader test.cpp)

target_compile_definitions(test_raytracer_reader PUBLIC SHAD_TASK_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")

if (TEST_SOLUTION)
    target_include_directories(test_raytracer_reader PUBLIC ../private/raytracer-geom)
else()
    target_include_directories(test_raytracer_reader PUBLIC ../raytracer-geom)
endif()
