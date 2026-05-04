#include "../scene.hpp"

struct aabb {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
    
    void grow(const glm::vec3 &p) {
        min = glm::min(min, p);
        max = glm::max(max, p);
    }
    
    void grow(const aabb &other) {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);
    }
    
    float surface_area() const {
        glm::vec3 d = max - min;
        return 2.0f * (d.x * d.y + d.y * d.z + d.z * d.x);
    }
};

struct tri_info {
    aabb bounds;
    glm::vec3 centroid;
};

struct bvh_builder {
    std::vector<bvh_node> nodes;
    std::vector<uint32_t> tri_indices; // reordered triangle indices
    const triangle *triangles;
    uint32_t num_triangles;
    std::vector<tri_info> tri_infos;
    
    bvh_builder(const triangle *tris, uint32_t count) 
        : triangles(tris), num_triangles(count) {
        tri_indices.resize(count);
        tri_infos.resize(count);
        
        for (uint32_t i = 0; i < count; i++) {
            tri_indices[i] = i;
            
            const auto &tri = tris[i];
            tri_infos[i].bounds.grow(tri.corners[0]);
            tri_infos[i].bounds.grow(tri.corners[1]);
            tri_infos[i].bounds.grow(tri.corners[2]);
            tri_infos[i].centroid = (tri.corners[0] + tri.corners[1] + tri.corners[2]) / 3.0f;
        }
        
        nodes.reserve(count * 2); // worst case
    }
    
    uint32_t build(uint32_t start, uint32_t end) {
        uint32_t node_idx = nodes.size();
        nodes.emplace_back();
        
        aabb bounds;
        aabb centroid_bounds;
        for (uint32_t i = start; i < end; i++) {
            bounds.grow(tri_infos[tri_indices[i]].bounds);
            centroid_bounds.grow(tri_infos[tri_indices[i]].centroid);
        }
        
        uint32_t count = end - start;
        
        // leaf node threshold
        if (count <= 4) {
            nodes[node_idx].aabb_min = bounds.min;
            nodes[node_idx].aabb_max = bounds.max;
            nodes[node_idx].left_idx_or_tri_begin = start | 0x80000000; // set leaf bit
            nodes[node_idx].right_idx_or_tri_count = count;
            return node_idx;
        }
        
        // find best split using SAH
        float best_cost = std::numeric_limits<float>::max();
        int best_axis = -1;
        float best_pos = 0.0f;
        
        const int num_bins = 16;
        struct bin {
            aabb bounds;
            uint32_t count = 0;
        };
        
        for (int axis = 0; axis < 3; axis++) {
            float axis_min = centroid_bounds.min[axis];
            float axis_max = centroid_bounds.max[axis];
            
            if (axis_min == axis_max) continue; // all centroids same on this axis
            
            bin bins[num_bins];
            
            // populate bins
            float scale = num_bins / (axis_max - axis_min);
            for (uint32_t i = start; i < end; i++) {
                uint32_t tri_idx = tri_indices[i];
                float centroid = tri_infos[tri_idx].centroid[axis];
                int bin_idx = std::min(num_bins - 1, 
                    (int)((centroid - axis_min) * scale));
                bins[bin_idx].count++;
                bins[bin_idx].bounds.grow(tri_infos[tri_idx].bounds);
            }
            
            // evaluate split positions between bins
            float left_areas[num_bins - 1];
            float right_areas[num_bins - 1];
            uint32_t left_counts[num_bins - 1];
            uint32_t right_counts[num_bins - 1];
            
            aabb left_box, right_box;
            uint32_t left_sum = 0, right_sum = 0;
            
            for (int i = 0; i < num_bins - 1; i++) {
                left_sum += bins[i].count;
                left_box.grow(bins[i].bounds);
                left_counts[i] = left_sum;
                left_areas[i] = left_box.surface_area();
            }
            
            for (int i = num_bins - 1; i > 0; i--) {
                right_sum += bins[i].count;
                right_box.grow(bins[i].bounds);
                right_counts[i - 1] = right_sum;
                right_areas[i - 1] = right_box.surface_area();
            }
            
            // find minimum cost split
            for (int i = 0; i < num_bins - 1; i++) {
                float cost = left_counts[i] * left_areas[i] + 
                             right_counts[i] * right_areas[i];
                
                if (cost < best_cost) {
                    best_cost = cost;
                    best_axis = axis;
                    best_pos = axis_min + (axis_max - axis_min) * (i + 1) / num_bins;
                }
            }
        }
        
        // check if split is worth it
        float leaf_cost = count * bounds.surface_area();
        if (best_cost >= leaf_cost || best_axis == -1) {
            // make leaf
            nodes[node_idx].aabb_min = bounds.min;
            nodes[node_idx].aabb_max = bounds.max;
            nodes[node_idx].left_idx_or_tri_begin = start | 0x80000000;
            nodes[node_idx].right_idx_or_tri_count = count;
            return node_idx;
        }
        
        // partition triangles
        uint32_t mid = start;
        for (uint32_t i = start; i < end; i++) {
            if (tri_infos[tri_indices[i]].centroid[best_axis] < best_pos) {
                std::swap(tri_indices[i], tri_indices[mid]);
                mid++;
            }
        }
        
        // handle edge case where all went to one side
        if (mid == start || mid == end) {
            mid = (start + end) / 2;
        }
        
        // build children
        uint32_t left_child = build(start, mid);
        uint32_t right_child = build(mid, end);
        
        nodes[node_idx].aabb_min = bounds.min;
        nodes[node_idx].aabb_max = bounds.max;
        nodes[node_idx].left_idx_or_tri_begin = left_child;
        nodes[node_idx].right_idx_or_tri_count = right_child;
        
        return node_idx;
    }
};

void scene::build_bvh() {
    std::cout << "build_bvh" << std::endl;

    bvh_builder builder(this->triangles, this->num_triangles);
    builder.build(0, this->num_triangles);
    
    // reorder triangles and ext based on builder's ordering
    auto *new_triangles = new triangle[this->num_triangles];
    auto *new_triangles_ext = new triangle_ext[this->num_triangles];
    
    for (uint32_t i = 0; i < this->num_triangles; i++) {
        uint32_t old_idx = builder.tri_indices[i];
        new_triangles[i] = this->triangles[old_idx];
        new_triangles_ext[i] = this->triangles_ext[old_idx];
    }
    
    delete[] this->triangles;
    delete[] this->triangles_ext;
    this->triangles = new_triangles;
    this->triangles_ext = new_triangles_ext;
    
    // copy nodes
    this->num_bvh_nodes = builder.nodes.size();
    this->bvh_nodes = new bvh_node[this->num_bvh_nodes];
    memcpy(this->bvh_nodes, builder.nodes.data(), this->num_bvh_nodes * sizeof(bvh_node));
}
