#include <common/io.hpp>

#include "app.hpp"
#include "shaders.hpp"

// Shader kernels
$entrypoint(vertex)
{
	// Vertex inputs
	layout_in <vec3> position(0);

	// Stage outputs
	layout_out <vec3> out_position(0);

	// Projection information
	push_constant <MVP> mvp;

	// Projecting the vertex
	gl_Position = mvp.project(position);

	// Regurgitate positions
	out_position = position;
};

$entrypoint(fragment)
{
	layout_in <vec3> position(0);

	// Resulting fragment color
	layout_out <vec4> fragment(0);

	// Render the normal vectors
	vec3 dU = dFdxFine(position);
	vec3 dV = dFdyFine(position);
	vec3 N = normalize(cross(dV, dU));

	fragment = vec4(0.5f + 0.5f * N, 1.0f);
};

// Pipeline compilation
void Application::compile_pipeline()
{
	auto [binding, attributes] = binding_and_attributes(VertexFlags::ePosition);

	auto vertex_spv = link(vertex).generate(Target::spirv_binary_via_glsl, Stage::vertex);
	auto fragment_spv = link(fragment).generate(Target::spirv_binary_via_glsl, Stage::fragment);

	// TODO: automatic generation by observing used layouts
	auto bundle = littlevk::ShaderStageBundle(resources.device, resources.dal)
		.code(vertex_spv.as <BinaryResult> (), vk::ShaderStageFlagBits::eVertex)
		.code(fragment_spv.as <BinaryResult> (), vk::ShaderStageFlagBits::eFragment);

	raster = littlevk::PipelineAssembler <littlevk::PipelineType::eGraphics>
		(resources.device, resources.window, resources.dal)
		.with_render_pass(render_pass, 0)
		.with_vertex_binding(binding)
		.with_vertex_attributes(attributes)
		.with_shader_bundle(bundle)
		.with_push_constant <solid_t <MVP>> (vk::ShaderStageFlagBits::eVertex, 0)
		.with_push_constant <solid_t <u32>> (vk::ShaderStageFlagBits::eFragment, sizeof(solid_t <MVP>))
		.cull_mode(vk::CullModeFlagBits::eNone);
}

// Debugging
void Application::shader_debug()
{
	set_trace_destination(root() / ".javelin");

	trace_unit("normals", Stage::vertex, vertex);
	trace_unit("normals", Stage::vertex, fragment);
}
