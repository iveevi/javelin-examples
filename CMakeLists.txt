cmake_minimum_required(VERSION 3.30)

project(javelin-examples C CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(OpenMP REQUIRED)

add_subdirectory(thirdparty/javelin)

include_directories(.
	thirdparty
	thirdparty/imgui
	thirdparty/glm
	thirdparty/argparse/include
	thirdparty/freetype/include
	thirdparty/javelin/include
	thirdparty/tinyobjloader)

add_subdirectory(common)

add_subdirectory(colored)
add_subdirectory(compute)
add_subdirectory(font)
add_subdirectory(material)
add_subdirectory(meshlet)
add_subdirectory(ngf)
add_subdirectory(normals)
add_subdirectory(palette)
add_subdirectory(particles)
add_subdirectory(pathtracing)
add_subdirectory(raytracing)

# TODO: function to generate exes

# Prototype example
add_executable(prototype prototype/main.cpp)
target_link_libraries(prototype PRIVATE javelin common)