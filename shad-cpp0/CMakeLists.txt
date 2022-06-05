project(shad-cpp0)

cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD             17)
set(CMAKE_MODULE_PATH              "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS  ON)

find_package(Catch REQUIRED)
find_package(PNG)
find_package(JPEG)

find_package(Poco QUIET COMPONENTS Foundation Net JSON)
if (NOT Poco_FOUND)
  message(STATUS "Seems like POCO is not installed on your machine.")
  message(STATUS "Tasks that depend on POCO are disabled.")
endif()

find_package(PythonLibs 3 QUIET)
if (NOT PythonLibs_FOUND)
  message(STATUS "python3 libraries not found. Python tasks are disabled.")
endif()

include(cmake/TestSolution.cmake)
include(cmake/BuildFlags.cmake)

if (Poco_FOUND)
  add_subdirectory(bot)
endif()

if (PythonLibs_FOUND)
endif()

if (${PNG_FOUND} AND ${JPEG_FOUND})
  add_subdirectory(raytracer-debug)
  add_subdirectory(raytracer-geom)
  add_subdirectory(raytracer-reader)
  add_subdirectory(raytracer)
elseif()
  message(STATUS "raytracer disabled. PNG:${PNG_FOUND} JPEG:${JPEG_FOUND}")
endif()

# my code start

add_executable(bot-main
        main.cpp
        bot/telegram/bot.cpp bot/telegram/bot.h bot/telegram/client.cpp bot/telegram/client.h bot/telegram/exceptions.cpp bot/telegram/exceptions.h
        raytracer/camera_options.h raytracer/commons.hpp raytracer/image.h raytracer/pre_image.h raytracer/render_options.h
        # raytracer-geom/vector.h raytracer-geom/triangle.h raytracer-geom/sphere.h raytracer-geom/ray.h raytracer-geom/intersection.h raytracer-geom/geometry.h
        # raytracer-reader/light.h raytracer-reader/material.h raytracer-reader/object.h raytracer-reader/scene.h
        )

target_link_libraries(bot-main ${PNG_LIBRARY} ${JPEG_LIBRARIES} PocoNet PocoNetSSL PocoFoundation PocoJSON)

if (TEST_SOLUTION)
  target_include_directories(bot-main PUBLIC private/raytracer-geom)
  target_include_directories(bot-main PUBLIC private/raytracer-reader)
else()
  target_include_directories(bot-main PUBLIC raytracer-geom)
  target_include_directories(bot-main PUBLIC raytracer-reader)
endif()

#include(bot/bot.cmake)
#include(raytracer/raytracer.cmake)
#
#target_link_libraries(bot-main bot-main-bot bot-main-raytracer)

# my code stop
