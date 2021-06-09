#include "core/material.hpp"

using namespace math;

namespace core {

fvec3 material::get_albedo(const fvec2 &coord) const {
	fvec3 albedo = albedo_fac;
	if (albedo_tex)
		albedo *= fvec3(albedo_tex->sample(coord));
	return albedo;
}

float material::get_opacity(const fvec2 &coord) const {
	float opacity = opacity_fac;
	if (opacity_tex)
		opacity *= opacity_tex->sample(coord).w;
	return opacity;
}

float material::get_occlusion(const fvec2 &coord) const {
	float occlusion = 1;
	if (occlusion_tex)
		occlusion *= occlusion_tex->sample(coord).x;
	return occlusion;
}

float material::get_roughness(const fvec2 &coord) const {
	float roughness = roughness_fac;
	if (roughness_tex)
		roughness *= roughness_tex->sample(coord).y;
	return roughness;
}

float material::get_metallic(const fvec2 &coord) const {
	float metallic = metallic_fac;
	if (metallic_tex)
		metallic *= metallic_tex->sample(coord).z;
	return metallic;
}

fvec3 material::get_emissive(const fvec2 &coord) const {
	fvec3 emissive = emissive_fac;
	if (emissive_tex)
		emissive *= fvec3(emissive_tex->sample(coord));
	return emissive;
}

}