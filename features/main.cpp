// TODO: autodiff on inputs, for callables and shaders
// TODO: partial evaluation of callables through substitution methods
// TODO: external constant specialization
// TODO: atomics
// TODO: fix optimization...

#include <littlevk/littlevk.hpp>

#include <common/io.hpp>

#include "common/application.hpp"
#include "common/default_framebuffer_set.hpp"

#include <ire.hpp>
#include <rexec.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace jvl;
using namespace jvl::ire;
using namespace jvl::rexec;

MODULE(features);

// TODO: l-value propagation
// TODO: get line numbers for each invocation if possible?
// using source location if available

// TODO: for partial procedures, need to return
// rexec_procedure when specializing...

struct device_state_hash {
	// State encoded by actively bound resources
	void *renderpass = nullptr;
	void *pipeline = nullptr;
	// TODO: std::set if there are disjoint descriptors...
	// void *descriptor = nullptr;
	// TODO: singleton tracker per command buffer per thread
};

// Specialized descriptors for any group of resources
template <resource_bindable ... Args>
struct Descriptor {

};

// Pipelines types
template <
	typename VertexElement,		// Vertex buffer element
	typename VertexConstants,	// Push constants for vertex shader
	typename FragmentConstants,	// Push constants for fragment shader
	typename Bindables		// Other resources linked through descriptor sets
>
struct TraditionalPipeline {
};

template <typename Bindables>
struct MeshShaderPipeline {};

template <typename Bindables>
struct RayTracingPipeline {
};

// Pipeline-specific compiler
auto compile_traditional() {}

auto compile_mesh_shader() {}

auto compile_ray_tracing() {}

// General compiler
template <entrypoint_class ... Entrypoints>
auto compile(const Entrypoints &... entrypoints)
{
	// Delegates to pipeline-specific compilers depending
	// on the search for a stage specific entrypoint...

	// Traditional pipeline
	// 	-> identify by vertex shader
	// Mesh shader pipeline
	// 	-> identify by mesh shader
	// Ray tracing pipeline
	// 	-> identify by ray generation shader

	// TODO: check types, arrange however is necessary, etc..
}

//////////////////////////////////////////
// Command buffer dierctives as objects //
//////////////////////////////////////////

struct Begin {};
struct BeginRenderPass {};

struct BindPipeline {};

template <resource_bindable ... Resources>
struct BindDescriptorSet {};

template <typename VertexElement>
struct BindVertexBuffers {};

// TODO: parameterize by topology...
struct BindIndexBuffer {};
struct Draw {};
struct DrawInstanced {};

struct DrawMeshTasks {};
struct TraceRays {};

// Forward declaring command buffer states
struct cb_zero;
struct cb_active;
struct cb_render_pass;

// Once pipelines are bound...
template <resource_bindable ... Resources>
struct Mask {
	// Resources are the remaining resources
	// which need to be bound...
};

template <
	typename ReadyMask,		// Collections of unbounded resources
	typename VertexElement,
	typename VertexConstants,
	typename FragmentConstants,
	typename Bindables
>
struct CommandBufferTraditionalPipeline {
	TraditionalPipeline <
		VertexElement,
		VertexConstants,
		FragmentConstants,
		Bindables
	> &ppl;
};

template <typename ReadyMask, resource_bindable ... Args>
struct cb_ppl_mesh_shader {
	MeshShaderPipeline <Args...> &ppl;
};

template <typename ReadyMask, resource_bindable ... Args>
struct cb_ppl_ray_tracing {
	RayTracingPipeline <Args...> &ppl;
};

// Launching state for each kind of pipeline
// TODO: these are specializations with the ready mask fully set...
template <
	typename VertexElement,
	typename VertexConstants,
	typename FragmentConstants,
	typename Bindables
>
struct CommandBufferTraditionalPipeline <Mask <>, VertexElement, VertexConstants, FragmentConstants, Bindables> {
	vk::CommandBuffer handle;

	TraditionalPipeline <
		VertexElement,
		VertexConstants,
		FragmentConstants,
		Bindables
	> &ppl;

	// No transition of state here?


};

template <typename ... Args>
struct cb_ppl_mesh_shader <Mask <>, Args...> {
	vk::CommandBuffer handle;
	MeshShaderPipeline <Args...> &ppl;
};

template <typename ... Args>
struct cb_ppl_ray_tracing <Mask <>, Args...> {
	vk::CommandBuffer handle;
	RayTracingPipeline <Args...> &ppl;
};

// struct cb_ready_traditional;
// struct cb_ready_mesh_shader;
// struct cb_ready_ray_tracing;

// TODO: queues must only accept cb_completed
struct cb_completed;

struct cb_zero {
	vk::CommandBuffer handle;

	// Most capabilities are locked...
	// TODO: render pass starting...
};

struct cb_active {
	vk::CommandBuffer handle;
};

struct cb_in_render_pass {
	device_state_hash hash;
	vk::CommandBuffer handle;
};

struct cb_bound_pipeline {
	device_state_hash hash;
	vk::CommandBuffer handle;
};

// Shaders via REXECs
struct Inputs {
	vec3 iResolution;
	f32 iTime;

	auto layout() {
		return layout_from("Inputs",
			verbatim_field(iResolution),
			verbatim_field(iTime));
	}
};

$rexec_vertex(Quad, $layout_out(vec2, 0))
{
	$rexec_entrypoint(main) {
		array <vec4> locations = std::array <vec4, 6> {
			vec4(-1, -1, 0, 1),
			vec4(1, -1, 1, 1),
			vec4(-1, 1, 0, 0),
			vec4(-1, 1, 0, 0),
			vec4(1, -1, 1, 1),
			vec4(1, 1, 1, 0),
		};

		vec4 v = locations[gl_VertexIndex % 6];
		gl_Position = vec4(v.xy(), 0, 1);
		$lout(0) = v.zw();
	};
};

template <builtin T>
struct buffer_index <T> {
	static thunder::Index of(const T &value) {
		return value.synthesize().id;
	}
};

$rexec_fragment(Sunset, $layout_out(vec4, 0), $layout_in(vec2, 0), $push_constant(Inputs, 0))
{
	$rexec_subroutine(f32, noise, vec3 p) {
		vec3 i = floor(p);
		vec4 a = dot(i, vec3(1.0, 57.0, 21.0)) + vec4(0.0, 57.0, 21.0, 78.0);
		vec3 f = 0.5 - 0.5 * cos((p - i) * acos(-1.0));
		a = mix(sin(cos(a) * a),sin(cos(1.0 + a) * (1.0 + a)), f.x);
		// TODO: allow this to work... return a swizzle_splice with reference fields?
		// a.xy() = mix(a.xz(), a.yw(), f.y);
		vec2 tmp = mix(a.xz(), a.yw(), f.y);
		a.x = tmp.x;
		a.y = tmp.y;
		
		auto &em = Emitter::active;
		em.materialize(i);
		em.materialize(a);
		em.materialize(f);
		em.materialize(tmp);

		$return mix(a.x, a.y, f.z);
	};

	$rexec_subroutine(f32, sphere, vec3 p, vec4 spr)
	{
		$return length(spr.xyz() - p) - spr.w;
	};

	$rexec_subroutine(f32, flame, vec3 p)
	{
		f32 d = sphere(p * vec3(1.0, 0.5, 1.0), vec4(0.0, -1.0, 0.0, 1.0));
		$return d + (noise(p + vec3(0.0, $constants.iTime * 2.0, 0.0)) + noise(p * 3.0) * 0.5) * 0.25 * p.y;
	};

	$rexec_subroutine(f32, scene, vec3 p)
	{
		$return min(100.0 - length(p), abs(flame(p)));
	};

	$rexec_subroutine(vec4, raymarch, vec3 org, vec3 dir)
	{
		f32 d = 0.0;
		f32 glow = 0.0;
		f32 eps = 0.02;
		vec3  p = org;
		boolean glowed = false;
		
		$for (i, range(0, 64)) {
			d = scene(p) + eps;
			p += d * dir;
			$if (d > eps) {
				$if (flame(p) < 0.0) {
					glowed = true;
				};

				$if (glowed) {
					glow = f32(i) / 64.0;
				};
			};
		};

		$return vec4(p, glow);
	};

	// TODO: Shadertoy rexec?
	$rexec_subroutine(void, mainImage, out <vec4> fragColor, in <vec2> fragCoord) {
		vec2 v = -1.0 + 2.0 * fragCoord.xy() / $constants.iResolution.xy();
		v.x *= $constants.iResolution.x / $constants.iResolution.y;
		
		vec3 org = vec3(0.0, -2.0, 4.0);
		vec3 dir = normalize(vec3(v.x * 1.6f, -v.y, -1.5));
		
		vec4 p = raymarch(org, dir);
		f32 glow = p.w;
		
		vec4 col = mix(vec4(1.0, 0.5, 0.1, 1.1), vec4(0.1, 0.5, 1.0, 1.0), p.y * 0.02f + 0.4);
		
		fragColor = mix(vec4(0.0), col, pow(glow * 2.0, 4.0));
	};

	$rexec_entrypoint(main) {
		mainImage($lout(0), $lin(0) * $constants.iResolution.xy() + 0.5);
	};
};

struct Application : BaseApplication {
	vk::RenderPass renderpass;
	littlevk::Pipeline pipeline;
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

		auto bundle = littlevk::ShaderStageBundle(resources.device, resources.dal)
			.code(vspv.as <BinaryResult> (), vk::ShaderStageFlagBits::eVertex);

		if (program["fallback"] == true)
			bundle.file("features/fire.frag", vk::ShaderStageFlagBits::eFragment);
		else
			bundle.code(fspv.as <BinaryResult> (), vk::ShaderStageFlagBits::eFragment);

		pipeline = littlevk::PipelineAssembler <littlevk::PipelineType::eGraphics>
			(resources.device, resources.window, resources.dal)
			.with_render_pass(renderpass, 0)
			.with_shader_bundle(bundle)
			.with_push_constant <solid_t <Inputs>> (vk::ShaderStageFlagBits::eFragment)
			.cull_mode(vk::CullModeFlagBits::eNone);
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
