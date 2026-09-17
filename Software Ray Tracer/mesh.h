#ifndef MESH_H
#define MESH_H

class mesh : public hittable
{
public:
	mesh() {}

	mesh(const hittable_list& triangles) : triangle_list(triangles)
	{
		// Wrap all the triangles for this specific shape into a BVH node
		bvh = make_shared<bvh_node>(triangles);

		for (const auto& triangle : triangles.objects) {
			area += triangle->get_area();

			cdf.push_back(area);
		}
	}

	virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const override 
	{
		// The mesh just delegates the hit test to its internal BVH
		return bvh->hit(r, ray_t, rec);
	}

	float pdf_value(const point3& origin, const glm::vec3& direction) const override {
		hit_record rec;
		if (!this->hit(ray(origin, direction), interval(0.001, infinity), rec))
			return 0;

		auto distance_squared = rec.t * rec.t * dot(direction, direction);
		auto cosine = std::fabs(dot(direction, rec.normal) / direction.length());

		auto denominator = cosine * area;
		if (denominator < 0.000001f) return 0.0f; // Return 0 probability instead of Inf
		return distance_squared / denominator;
	}

	glm::vec3 random(const point3& origin) const override {
		if (cdf.empty()) return glm::vec3(1, 0, 0); // Fallback for empty mesh

		// --- Implementation for random point generation on the mesh ---

		// For simplicity, we could randomly select a triangle and then generate a random point on that triangle.
		// but it can be incorrect because it does not take into account the area of each triangle.

		// The more accurate approach is to select a triangle based on its area relative to the total area of the mesh.
		float r = random_float(0.0f, area);

		auto it = std::lower_bound(cdf.begin(), cdf.end(), r); // search for the first triangle whose cumulative area 
		int index = std::distance(cdf.begin(), it);

		if (index >= triangle_list.objects.size()) 
		{
			index = triangle_list.objects.size() - 1;
		}

		return triangle_list.objects[index]->random(origin);
	}

	aabb bounding_box() const override 
	{
		return bvh->bounding_box();
	}

private:
	shared_ptr<hittable> bvh;
	std::vector<float> cdf;
	hittable_list triangle_list;
	float area = 0.0f;
};


#endif