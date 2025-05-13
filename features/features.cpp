// TODO: autodiff on inputs, for callables and shaders
// TODO: partial evaluation of callables through substitution methods
// TODO: external constant specialization
// TODO: atomics
// TODO: fix optimization...

#include <littlevk/littlevk.hpp>

#include <common/io.hpp>

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

struct SimulationInfo {
	i32 elements;
	f32 delta;
	unsized_array <vec3> positions;

	auto layout() {
		return layout_from("SimulationInfo",
			verbatim_field(elements),
			verbatim_field(delta),
			verbatim_field(positions));
	}
};

static_assert(aggregate <SimulationInfo>);
static_assert(!solidifiable <SimulationInfo>);

int main()
{
	auto sbe = soft_t <SimulationInfo> ();
	sbe.get <0> () = 1204;
	sbe.get <1> () = 13.0f;
	fmt::println("count: {}", sbe.count());
	sbe.push(glm::vec3(10.0f));
	fmt::println("count: {}", sbe.count());
	sbe.extend({ glm::vec3(2.0f), glm::vec3(4.0f) });
	fmt::println("count: {}", sbe.count());
	fmt::println("[0] = ({}, {}, {})", sbe[0].x, sbe[0].y, sbe[0].z);
	fmt::println("[1] = {}", glm::to_string((glm::vec3) sbe[1]));
	fmt::println("[2] = {}", glm::to_string((glm::vec3) sbe[2]));
}
