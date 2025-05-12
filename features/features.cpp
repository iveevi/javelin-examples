// TODO: autodiff on inputs, for callables and shaders
// TODO: partial evaluation of callables through substitution methods
// TODO: external constant specialization
// TODO: atomics
// TODO: fix optimization...

#include <littlevk/littlevk.hpp>

#include <common/io.hpp>

#include <ire.hpp>
#include <rexec.hpp>

using namespace jvl;
using namespace jvl::ire;
using namespace jvl::rexec;

MODULE(features);

// TODO: type check for soft/concrete types... layout_from only for concrete types
// TODO: generic is anything,
// 	but solid_t aggregates must take only concrete_generics
//      and soft_t aggregates can take anything, defaults to solid_t is all concrete

// TODO: l-value propagation
// TODO: get line numbers for each invocation if possible?
// using source location if available

// TODO: for partial procedures, need to return
// rexec_procedure when specializing...

enum Textures {
	eAlbedo,
	eSpecular,
	eNormal,
	eRoughness,
	eCount,
};

struct Tmp {
	i32 mode;
	static_array <vec3, eCount> sampled;

	auto layout() {
		return layout_from("Tmp",
			verbatim_field(mode),
			verbatim_field(sampled));
	}
};

$subroutine(Tmp, sample, vec2 uv)
{
	Tmp result;

	result.mode = 12;

	for (uint32_t i = 0; i < eCount; i++) {
		sampler2D tex(i);

		result.sampled[i] = tex.sample(uv).xyz();
	}

	$return result;
};

$subroutine(vec3, eval)
{
	push_constant <Tmp> constants;

	vec3 sum = vec3(0);
	for (uint32_t i = 0; i < eCount; i++)
		sum += constants.sampled[i];

	$return sum / f32(eCount);
};

static_assert(is_solidifiable_layout((Layout <false, field <i32>, field <static_array <vec3, 4>>> *) nullptr));
static_assert(aggregate <Tmp>);

// using Bytes = std::vector <uint8_t>;

// template <padded_type ... Ts>
// struct soft_padded : Bytes {
// 	// TODO: split into solid and last type...
// };

int main()
{
	auto tmp = solid_t <Tmp> ();
	tmp.get <0> () = 1;
	tmp.get <1> ()[2] = glm::vec3(0.0);

	fmt::println("{}", link(sample).generate_glsl());
	fmt::println("{}", link(eval).generate_glsl());
}
