#pragma once

#include "pch.hpp"

#include "math/mat3.h"
#include "math/quat.h"
#include "math/vec3.h"
#include "util/strings.h"

namespace math {

struct transform {
	static const transform identity;

	fvec3 origin;
	fmat3 basis;

	transform(const fvec3 &origin = fvec3(0),
			  const fmat3 &basis = fmat3(1));

	static transform make(
			const fvec3 &translation = fvec3(0),
			const quat &rotation = quat(1),
			const fvec3 &scale = fvec3(1));

	static fmat3 make_basis(
			const quat &rotation = quat(1),
			const fvec3 &scale = fvec3(1));

	transform inverse() const;

	quat get_rotation() const;

	void set_rotation(const quat &rotation);

	void rotate(const quat &rotation);

	fvec3 get_scale() const;

	void set_scale(const fvec3 &scale);

	void scale(const fvec3 &scale);

	transform operator*(const transform &rhs) const;

	friend std::ostream &operator<<(std::ostream &lhs, const transform &rhs);
};

}
