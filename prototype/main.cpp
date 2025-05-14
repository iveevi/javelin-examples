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

// Strongly typed descriptor sets
template <resource_bindable ... Resources>
struct Descriptor {};

// Pending resource lists
template <resource_indicator ... Resources>
struct Pending {};

// TODO: static render pass and framebuffer!

///////////////////////////////////////////
// Objectified command buffer directives //
///////////////////////////////////////////

// Managing commands buffers
struct BeginCommands {};
struct EndCommands {};

// Managing rendeirng passes
// TOOD: templated
// NOTE: this is a rexec as well in the sense
// than clear values must be provided
struct BeginRenderPass {
	const vk::RenderPass &renderpass;
	const vk::Framebuffer &framebuffer;
	vk::Extent2D extent;
	std::vector <vk::ClearValue> clears;
};

struct EndRenderPass {};

// Managing pipelines
template <pipeline_class T>
struct BindPipeline : T {};

struct UnbindPipeline {};

// Context (or state) aware command buffer directives
template <typename T>
struct FragmentPushConstants {
	const T &value;
};

struct Draw {
	uint32_t vertices;
	uint32_t instances;
};

////////////////////////////
// Phased command buffers //
////////////////////////////

// Contains any handles necessary to track state;
// ideally, overall changes in state are minimized

struct CommandBufferZero {
	vk::CommandBuffer handle;
};

struct CommandBufferActive {
	vk::CommandBuffer handle;
};

struct CommandBufferCompleted {
	vk::CommandBuffer handle;
};

// TODO: templated with render pass rexec
struct CommandBufferRenderPass {
	vk::CommandBuffer handle;
};

// TODO: templated wih resources and mask...
struct CommandBufferPipeline {
	vk::CommandBuffer handle;
	vk::PipelineLayout layout;
};

// Implementing execution transfers per directive
CommandBufferActive operator|(const CommandBufferZero &cmd, const BeginCommands &)
{
	cmd.handle.begin(vk::CommandBufferBeginInfo());
	return CommandBufferActive(cmd.handle);
}

CommandBufferCompleted operator|(const CommandBufferActive &cmd, const EndCommands &)
{
	cmd.handle.end();
	return CommandBufferCompleted(cmd.handle);
}

CommandBufferRenderPass operator|(const CommandBufferActive &cmd, const BeginRenderPass &brp)
{
	auto scissor = vk::Rect2D()
		.setExtent(brp.extent)
		.setOffset({ 0, 0 });
	
	auto viewport = vk::Viewport()
		.setWidth(brp.extent.width)
		.setHeight(brp.extent.height)
		.setMaxDepth(1.0)
		.setMinDepth(0.0)
		.setX(0)
		.setY(0);
	
	cmd.handle.setScissor(0, scissor);
	cmd.handle.setViewport(0, viewport);

	auto info = vk::RenderPassBeginInfo()
		.setRenderPass(brp.renderpass)
		.setFramebuffer(brp.framebuffer)
		.setRenderArea(scissor)
		.setClearValues(brp.clears);

	cmd.handle.beginRenderPass(info, vk::SubpassContents::eInline);

	return CommandBufferRenderPass(cmd.handle);
}

CommandBufferActive operator|(const CommandBufferRenderPass &cmd, const EndRenderPass &)
{
	cmd.handle.endRenderPass();
	return CommandBufferActive(cmd.handle);
}

template <pipeline_class T>
CommandBufferPipeline operator|(const CommandBufferRenderPass &cmd, const BindPipeline <T> &bind)
{
	// TODO: depends on the kind...
	cmd.handle.bindPipeline(vk::PipelineBindPoint::eGraphics, bind.handle);
	return CommandBufferPipeline(cmd.handle, bind.layout);
}

// TODO: each draw should reset pending list: but we should be
// smart with detecting already bound resources...
CommandBufferPipeline operator|(const CommandBufferPipeline &cmd, const Draw &draw)
{
	cmd.handle.draw(draw.vertices, draw.instances, 0, 0);
	// TODO: change this!
	return cmd;
}

// TODO: infer offset automatically...
template <typename T>
CommandBufferPipeline operator|(const CommandBufferPipeline &cmd, const FragmentPushConstants <T> &constants)
{
	cmd.handle.pushConstants <T> (cmd.layout,
		vk::ShaderStageFlagBits::eFragment,
		0, constants.value);
	return cmd;
}

CommandBufferRenderPass operator|(const CommandBufferPipeline &cmd, const UnbindPipeline &)
{
	return CommandBufferRenderPass(cmd.handle);
}

// Rendering application
int main()
{
	// TODO: switch to oak...
	auto extensions = std::vector <const char *> {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	// littlevk configuration
	{
		auto &config = littlevk::config();
		config.enable_logging = false;
		config.enable_validation_layers = true;
		config.abort_on_validation_error = true;
	}

	// Find a suitable physical device
	auto predicate = [&](vk::PhysicalDevice phdev) {
		return littlevk::physical_device_able(phdev, extensions);
	};

	auto phdev = littlevk::pick_physical_device(predicate);

	// Initializethe resources
	auto resources = VulkanResources::from(phdev,
		"Prototype",
		vk::Extent2D(1920, 1080),
		extensions);
	
	auto &device = resources.device;
	auto &window = resources.window;
	auto &swapchain = resources.swapchain;
	auto &command_pool = resources.command_pool;
	auto &deallocator = resources.dal;
	
	// Configuring rendering resources
	vk::RenderPass renderpass = littlevk::RenderPassAssembler
			(resources.device, resources.dal)
		.add_attachment(littlevk::default_color_attachment(resources.swapchain.format))
		.add_subpass(vk::PipelineBindPoint::eGraphics)
			.color_attachment(0, vk::ImageLayout::eColorAttachmentOptimal)
			.done();

	auto generator = littlevk::FramebufferGenerator(device,
		renderpass,
		window.extent,
		deallocator);

	for (size_t i = 0; i < resources.swapchain.images.size(); i++)
		generator.add(swapchain.image_views[i]);

	auto framebuffers = generator.unpack();

	// Compile pipelines
	// TODO: benchmark randomly generate
	// shaders with various optimizations
	// pass only if the stable variant is good
	// (use fixed seeds for repro)
	thunder::Optimizer::stable.apply(Sunset::noise);
	thunder::Optimizer::stable.apply(Sunset::sphere);
	thunder::Optimizer::stable.apply(Sunset::flame);
	thunder::Optimizer::stable.apply(Sunset::scene);
	thunder::Optimizer::stable.apply(Sunset::raymarch);
	thunder::Optimizer::stable.apply(Sunset::mainImage);
	thunder::Optimizer::stable.apply(Sunset::main);

	auto shadertoy = compile(device, renderpass, Sunset::main, Quad::main);

	// Rendering operation; formally is (CommandBufferZero, ...) -> CommandBufferCompleted
	auto render = [&](const CommandBufferZero &zero, uint32_t index) -> CommandBufferCompleted {
		static const vk::ClearValue clear = vk::ClearColorValue()
			.setFloat32({ 0, 0, 0, 0 });

		// Preparing constants data
		auto inputs = solid_t <Inputs> ();

		inputs.get <0> () = glm::vec3 {
			window.extent.width,
			window.extent.height,
			1.0,
		};

		inputs.get <1> () = glfwGetTime();

		// // Rendering commands
		// auto active = zero | BeginCommands();
		// auto rendering = active | BeginRenderPass(renderpass, framebuffers[index], window.extent, { clear });
		// auto pipeline = rendering | BindPipeline(shadertoy);
		// pipeline = pipeline | FragmentPushConstants(inputs) | Draw(6, 1);
		// active = rendering | EndRenderPass();
		// return active | EndCommands();

		return zero
			| BeginCommands()
				| BeginRenderPass(renderpass, framebuffers[index], window.extent, { clear })
					| BindPipeline(shadertoy)
						| FragmentPushConstants(inputs)
						| Draw(6, 1)
					| UnbindPipeline()
				| EndRenderPass()
			| EndCommands();
	};

	// Resizing operation
	auto resize = []() {

	};

	// Rendering loop
	uint32_t count = swapchain.images.size();

	auto sync = littlevk::present_syncronization(device, count).unwrap(deallocator);

	auto command_buffers = device.allocateCommandBuffers({
		command_pool,
		vk::CommandBufferLevel::ePrimary,
		count
	});

	Timer timer;

	uint32_t frame = 0;
	uint32_t total = 0;

	while (!glfwWindowShouldClose(window.handle)) {
		timer.reset();

		glfwPollEvents();

		littlevk::SurfaceOperation op;
		op = littlevk::acquire_image(device, swapchain.handle, sync[frame]);
		if (op.status == littlevk::SurfaceOperation::eResize) {
			resize();
			continue;
		}

		// Record the command buffer
		const auto &cmd = command_buffers[frame];

		render(CommandBufferZero(cmd), op.index);

		// cmd.begin(vk::CommandBufferBeginInfo {});
		// 	render(cmd, op.index, total);
		// cmd.end();

		// Submit command buffer while signaling the semaphore
		constexpr vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

		vk::SubmitInfo submit_info {
			sync.image_available[frame],
			wait_stage, cmd,
			sync.render_finished[frame]
		};

		resources.graphics_queue.submit(submit_info, sync.in_flight[frame]);

		op = littlevk::present_image(resources.present_queue, swapchain.handle, sync[frame], op.index);
		if (op.status == littlevk::SurfaceOperation::eResize)
			resize();

		frame = (++total) % count;
	}

	device.waitIdle();
	device.freeCommandBuffers(command_pool, command_buffers);
}