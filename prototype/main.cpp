// TODO: autodiff on inputs, for callables and shaders
// TODO: partial evaluation of callables through substitution methods
// TODO: external constant specialization
// TODO: atomics
// TODO: fix optimization...
// TODO: l-value propagation
// TODO: get line numbers for each invocation if possible? using source location if available
// TODO: for partial procedures, need to return rexec_procedure when specializing...

#include <littlevk/littlevk.hpp>

#include <common/io.hpp>

#include "common/application.hpp"
#include "common/default_framebuffer_set.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "shaders.hpp"

// Unbounded resources "mask"
template <resource_bindable ... Resources>
struct Pending {
	// Resources are the remaining resources
	// which need to be bound...
};

// Specialized descriptors for any subset of resources
template <resource_bindable ... Args>
struct Descriptor {

};

struct Application : BaseApplication {
	$pipeline(Quad, Sunset) pipeline;

	vk::RenderPass renderpass;
	DefaultFramebufferSet <false> framebuffers;

	Application() : BaseApplication("Features", {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			}, nullptr, nullptr, false) {
		renderpass = littlevk::RenderPassAssembler(resources.device, resources.dal)
			.add_attachment(littlevk::default_color_attachment(resources.swapchain.format))
			.add_attachment(littlevk::default_depth_attachment())
			.add_subpass(vk::PipelineBindPoint::eGraphics)
				.color_attachment(0, vk::ImageLayout::eColorAttachmentOptimal)
				.depth_attachment(1, vk::ImageLayout::eDepthStencilAttachmentOptimal)
				.done();

		framebuffers.resize(resources, renderpass);
	}

	void configure(argparse::ArgumentParser &program) override {
		program.add_argument("--fallback")
			.help("use GLSL fallback fragment shader")
			.default_value(false)
			.flag();
	}

	void preload(const argparse::ArgumentParser &program) override {
		thunder::Optimizer::stable.apply(Sunset::noise);
		thunder::Optimizer::stable.apply(Sunset::sphere);
		thunder::Optimizer::stable.apply(Sunset::flame);
		thunder::Optimizer::stable.apply(Sunset::scene);
		thunder::Optimizer::stable.apply(Sunset::raymarch);
		thunder::Optimizer::stable.apply(Sunset::mainImage);
		thunder::Optimizer::stable.apply(Sunset::main);
	
		auto vspv = link(Quad::main).generate(Target::spirv_binary_via_glsl, Stage::vertex);
		auto fspv = link(Sunset::main).generate(Target::spirv_binary_via_glsl, Stage::fragment);

		set_trace_destination("debug");

		trace_unit("sunset", Stage::fragment, Sunset::main);
		
		pipeline = compile(resources.device, renderpass, Quad::main, Sunset::main);
	}

	void render(const vk::CommandBuffer &cmd, uint32_t index, uint32_t) override {
		// Configure the rendering extent
		littlevk::viewport_and_scissor(cmd, littlevk::RenderArea(resources.window.extent));

		littlevk::RenderPassBeginInfo(2)
			.with_render_pass(renderpass)
			.with_framebuffer(framebuffers[index])
			.with_extent(resources.window.extent)
			.clear_color(0, std::array <float, 4> { 0, 0, 0, 0 })
			.clear_depth(1, 1)
			.begin(cmd);
	
		auto inputs = solid_t <Inputs> ();

		inputs.get <0> () = glm::vec3 {
			resources.window.extent.width,
			resources.window.extent.height,
			1.0,
		};

		inputs.get <1> () = glfwGetTime();
		
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.handle);
		cmd.pushConstants <solid_t <Inputs>> (pipeline.layout,
			vk::ShaderStageFlagBits::eFragment,
			0, inputs);
		cmd.draw(6, 1, 0, 0);
	
		cmd.endRenderPass();
	}

	void resize() override {

	}
};

APPLICATION_MAIN()
