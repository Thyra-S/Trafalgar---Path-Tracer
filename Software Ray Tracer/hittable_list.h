#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "aabb.h"
#include "hittable.h"

class hittable_list : public hittable
{
public:
	std::vector<shared_ptr<hittable>> objects;

	hittable_list() {}
	hittable_list(shared_ptr<hittable> object) { add(object); }

	void clear() { objects.clear(); }

	void add(shared_ptr<hittable> object)
	{
		objects.push_back(object);
		bbox = aabb(bbox, object->bounding_box());
	}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override
	{
		hit_record temp_rec;
		bool hit_anything = false;
		auto closest_so_far = ray_t.max;

		for (const auto& object : objects) {
			if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
				hit_anything = true;
				closest_so_far = temp_rec.t;
				rec = temp_rec;
			}
		}

		return hit_anything;
	}

	float pdf_value(const point3& origin, const glm::vec3& direction) const override {
		float weight = 1.0f / objects.size();
		float sum = 0.0f;
		for (const auto& object : objects) {
			sum += weight * object->pdf_value(origin, direction);
		}
		return sum;
	}

	glm::vec3 random(const point3& origin) const override {
		int index = random_int(0, objects.size() - 1);
		return objects[index]->random(origin);
	}

	aabb bounding_box() const override { return bbox; }

private:
	aabb bbox;
};

#endif