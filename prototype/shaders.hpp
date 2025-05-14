#pragma once

#include <ire.hpp>
#include <rexec.hpp>

using namespace jvl;
using namespace jvl::ire;
using namespace jvl::rexec;

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

$rexec_fragment(Sunset, $layout_out(vec4, 0), $layout_in(vec2, 0), $push_constant(Inputs, 0))
{
	$rexec_subroutine(f32, noise, vec3 p)
	{
		vec3 i = floor(p);
		vec4 a = dot(i, vec3(1.0, 57.0, 21.0)) + vec4(0.0, 57.0, 21.0, 78.0);
		vec3 f = 0.5 - 0.5 * cos((p - i) * acos(-1.0));
		a = mix(sin(cos(a) * a),sin(cos(1.0 + a) * (1.0 + a)), f.x);
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

	$rexec_entrypoint(main)
	{
		mainImage($lout(0), $lin(0) * $constants.iResolution.xy() + 0.5);
	};
};