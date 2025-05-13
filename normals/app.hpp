#pragma once

#include "common/application.hpp"
#include "common/vulkan_triangle_mesh.hpp"
#include "common/default_framebuffer_set.hpp"

struct Application : CameraApplication {
	littlevk::Pipeline raster;
	vk::RenderPass render_pass;
	DefaultFramebufferSet <true> framebuffers;

	Transform model_transform;

	glm::vec3 min;
	glm::vec3 max;
	bool automatic;

	std::vector <VulkanTriangleMesh> meshes;

	Application();
	~Application();

	void compile_pipeline();
	void shader_debug();

	void configure(argparse::ArgumentParser &program) override;

	void preload(const argparse::ArgumentParser &program) override;

	void render(const vk::CommandBuffer &cmd, uint32_t index, uint32_t total) override;
	void resize() override;
};