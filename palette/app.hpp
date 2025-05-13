#pragma once

#include "common/default_framebuffer_set.hpp"
#include "common/transform.hpp"
#include "common/vulkan_triangle_mesh.hpp"
#include "common/application.hpp"

struct Application : CameraApplication {
	littlevk::Pipeline traditional;
	vk::RenderPass render_pass;
	DefaultFramebufferSet <true> framebuffers;

	Transform model_transform;

	float saturation = 0.5f;
	float lightness = 1.0f;
	int splits = 16;

	glm::vec3 min;
	glm::vec3 max;
	bool automatic;

	std::vector <VulkanTriangleMesh> meshes;

	Application();
	~Application();

	void compile_pipeline(VertexFlags, float, float, int);
	void shader_debug();

	void configure(argparse::ArgumentParser &) override;
	void preload(const argparse::ArgumentParser &) override;

	void render(const vk::CommandBuffer &, uint32_t, uint32_t) override;
	void resize() override;
};