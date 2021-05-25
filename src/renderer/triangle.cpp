#include "renderer/triangle.hpp"

using namespace math;

namespace renderer {

triangle::triangle(
		const fvec3 &a,
		const fvec3 &b,
		const fvec3 &c
		) : a(a), b(b), c(c) {}

triangle::intersection triangle::intersect(const ray &ray) const {
	fvec3 ab = b - a;
	fvec3 bc = c - b;
	fvec3 ca = a - c;
	float denom;
	
	// We cross two vectors which lay on the triangle's plane
	// Could use -ca for the actual CCW-front normal
	fvec3 normal = cross(ab, ca);
	
	// If ray is perpendicular to the normal...
	denom = dot(normal, ray.get_dir());
	if (denom == 0)
		return { -1 };
	
	// d = proj(N, A - O) / proj(N, D)
	// ...where A is any point on the plane
	float dist = dot(normal, a - ray.origin) / denom;

	// If plane is behind the ray
	if (dist < 0)
		return { -1 };
	
	// This intersection point lies
	// on the same plane as our triangle
	// I = O + D * d
	fvec3 hit = ray.origin + ray.get_dir() * dist;

	// We need to find the barycentric
	// coordinates for that point
	// [alpha, beta, gamma]

	// Triangle heights for vertices A and B
	fvec3 ha = ab - proj(bc, ab);
	fvec3 hb = bc - proj(ca, bc);

	// alpha = 1 - (proj(ha, AI) / proj(ha, AB))
	// proj(ha, AB) = proj(ha, AC)
	// ha and AB/AC can only be perpendicular
	// for degenerate triangles
	denom = dot(ha, ab);
	if (is_approx(denom, 0))
		return { -1 };
	float alpha = 1 - (dot(ha, hit - a) / denom);

	// If any of the coordinates is negative,
	// intersection occured outside of the triangle
	if (alpha + epsilon < 0)
		return { -1 };

	denom = dot(hb, bc);
	if (is_approx(denom, 0))
		return { -1 };
	float beta = 1 - (dot(hb, hit - b) / denom);

	if (beta + epsilon < 0)
		return { -1 };

	float gamma = 1 - (alpha + beta);

	if (gamma + epsilon < 0)
		return { -1 };

	return { dist, fvec3(alpha, beta, gamma) };
}

}
