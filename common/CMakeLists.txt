# ImGui sources for GLFW + Vulkan
set(IMGUI_DIR ../thirdparty/imgui)

set(IMGUI_SOURCES
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
	${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp)

# Javelin graphics stack
add_library(common STATIC
	${IMGUI_SOURCES}
	acceleration_structure.cpp
	camera_controller.cpp
	font.cpp
	formats.cpp
	imgui.cpp
	imported_asset.cpp
	imported_mesh.cpp
	interactive_window.cpp
	material.cpp
	sbt.cpp
	texture.cpp
	texture_bank.cpp
	triangle_mesh.cpp
	vulkan_resources.cpp
	vulkan_triangle_mesh.cpp)

target_link_libraries(common PRIVATE glfw Vulkan::Vulkan SPIRV)

# TODO: use howler for logging here...
target_compile_definitions(common PRIVATE
	$<$<CONFIG:DEBUG>:JVL_DEBUG=1>)
