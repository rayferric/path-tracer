#include "renderer/model.hpp"

namespace renderer {

const std::vector<model::surface> &model::get_surfaces() const {
	return surfaces;
}

void model::add_surface(const surface &surface) {
	surfaces.push_back(surface);
}

}
