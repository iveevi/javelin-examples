// Adapted from https://www.shadertoy.com/view/MdX3zr
#version 460

layout (push_constant) uniform Constants
{
	vec3 iResolution;
	float iTime;
};

float noise(vec3 p)
{
	vec3 i = floor(p);
	vec4 a = dot(i, vec3(1.0, 57.0, 21.0)) + vec4(0.0, 57.0, 21.0, 78.0);
	vec3 f = 0.5 - 0.5 * cos((p - i) * acos(-1.0));
	a = mix(sin(cos(a) * a),sin(cos(1.0 + a) * (1.0 + a)), f.x);
	a.xy = mix(a.xz, a.yw, f.y);
	return mix(a.x, a.y, f.z);
}

float sphere(vec3 p, vec4 spr)
{
	return length(spr.xyz - p) - spr.w;
}

float flame(vec3 p)
{
	float d = sphere(p * vec3(1.0, 0.5, 1.0), vec4(0.0, -1.0, 0.0, 1.0));
	return d + (noise(p + vec3(0.0, iTime * 2.0, 0.0)) + noise(p * 3.0) * 0.5) *0.25 * (p.y);
}

float scene(vec3 p)
{
	return min(100.0 - length(p), abs(flame(p)));
}

vec4 raymarch(vec3 org, vec3 dir)
{
	float d = 0.0;
	float glow = 0.0;
	float eps = 0.02;
	vec3  p = org;
	bool glowed = false;
	
	for (int i = 0; i < 64; i++) {
		d = scene(p) + eps;
		p += d * dir;
		if (d > eps) {
			if (flame(p) < 0.0)
				glowed = true;
			if (glowed)
				glow = float(i) / 64.0;
		}
	}

	return vec4(p, glow);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 v = -1.0 + 2.0 * fragCoord.xy / iResolution.xy;
	v.x *= iResolution.x/iResolution.y;
	
	vec3 org = vec3(0.0, -2.0, 4.0);
	vec3 dir = normalize(vec3(v.x * 1.6, -v.y, -1.5));
	
	vec4 p = raymarch(org, dir);
	float glow = p.w;
	
	vec4 col = mix(vec4(1.0, 0.5, 0.1, 1.0), vec4(0.1, 0.5, 1.0, 1.0), p.y * 0.02 + 0.4);
	
	fragColor = mix(vec4(0.), col, pow(glow * 2.0, 4.0));
}

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 fragment;

void main()
{
	mainImage(fragment, uv * iResolution.xy + 0.5);
}