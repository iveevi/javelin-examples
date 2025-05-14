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

function(example target directory)
	file(GLOB target_source CONFIGURE_DEPENDS "${directory}/*.cpp")
	add_executable(${target} ${target_source})
	target_link_libraries(${target} PRIVATE javelin common ${ARGN})
endfunction()

example(ire-colored	"ire/colored")
example(ire-compute	"ire/compute")
example(ire-font	"ire/font"		freetype)
example(ire-material	"ire/material")
example(ire-meshlet	"ire/meshlet")
example(ire-ngf		"ire/ngf")
example(ire-normals	"ire/normals")
example(ire-palette	"ire/palette")
example(ire-particles	"ire/particles")
example(ire-pathtracing	"ire/pathtracing")
example(ire-raytracing	"ire/raytracing")

example(prototype	"prototype")