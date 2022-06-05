add_catch(test_raytracer_debug test.cpp)

target_compile_definitions(test_raytracer_debug PUBLIC SHAD_TASK_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")

if (GRADER)
  target_compile_definitions(test_raytracer_debug PUBLIC SHAD_ARTIFACTS_DIR="/tmp/artifacts")
endif()

if (TEST_SOLUTION)
    target_include_directories(test_raytracer_debug PUBLIC ../private/raytracer-geom)
    target_include_directories(test_raytracer_debug PUBLIC ../private/raytracer-reader)
    target_include_directories(test_raytracer_debug PUBLIC ../private/raytracer ../raytracer)
else()
    target_include_directories(test_raytracer_debug PUBLIC ../raytracer-geom)
    target_include_directories(test_raytracer_debug PUBLIC ../raytracer-reader)
    target_include_directories(test_raytracer_debug PUBLIC ../raytracer)
endif()

target_link_libraries(test_raytracer_debug ${PNG_LIBRARY} ${JPEG_LIBRARIES})
target_include_directories(
  test_raytracer_debug
  PRIVATE ${PNG_INCLUDE_DIRS} ${JPEG_INCLUDE_DIRS}
)
