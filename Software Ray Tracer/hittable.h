#ifndef HITTABLE_H
#define	HITTABLE_H	

#include "aabb.h"

class material;

class hit_record
{
public: 
	point3 p;
	glm::vec3 normal;
	shared_ptr<material> mat;
	float t;
	float u;
	float v;
	bool front_face;
	
	void set_face_normal(const ray& r, const glm::vec3& outward_normal)
	{
		front_face = glm::dot(r.direction(), outward_normal) < 0;
		normal = front_face ? outward_normal : -outward_normal;
	}
};
class hittable
{
public:
	virtual ~hittable() = default;

	virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;

	virtual aabb bounding_box() const = 0;

	virtual float pdf_value(const point3& origin, const glm::vec3& direction) const {
		return 0.0;
	}

	virtual glm::vec3 random(const point3& origin) const {
		return glm::vec3(1, 0, 0);
	}

	virtual float get_area() const {
		return 0.0;
	}
};

class translate : public hittable {
public:
	translate(shared_ptr<hittable> object, const glm::vec3& offset)
		: object(object), offset(offset) 
	{
		bbox = object->bounding_box() + offset;
	}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
		// Move the ray backwards by the offset
		ray offset_r(r.origin() - offset, r.direction(), r.time());

		// Determine whether an intersection exists along the offset ray (and if so, where)
		if (!object->hit(offset_r, ray_t, rec))
			return false;

		// Move the intersection point forwards by the offset
		rec.p += offset;

		return true;
	}

	float pdf_value(const point3& origin, const glm::vec3& direction) const override {
		return object->pdf_value(origin - offset, direction);
	}

	glm::vec3 random(const point3& origin) const override {
		return object->random(origin - offset);
	}

	aabb bounding_box() const override { return bbox; }

private:
	shared_ptr<hittable> object;
	glm::vec3 offset;
	aabb bbox;
};


class rotate_y : public hittable {
public:
	rotate_y(shared_ptr<hittable> object, float angle) : object(object) 
	{
		auto radians = degrees_to_radians(angle);
		sin_theta = std::sin(radians);
		cos_theta = std::cos(radians);
		bbox = object->bounding_box();

		point3 min(infinity, infinity, infinity);
		point3 max(-infinity, -infinity, -infinity);

		for (int i = 0; i < 2; i++) 
		{
			for (int j = 0; j < 2; j++) 
			{
				for (int k = 0; k < 2; k++) 
				{
					auto x = i * bbox.x.max + (1 - i) * bbox.x.min;
					auto y = j * bbox.y.max + (1 - j) * bbox.y.min;
					auto z = k * bbox.z.max + (1 - k) * bbox.z.min;

					auto newx = cos_theta * x + sin_theta * z;
					auto newz = -sin_theta * x + cos_theta * z;

					glm::vec3 tester(newx, y, newz);

					for (int c = 0; c < 3; c++) 
					{
						min[c] = std::fmin(min[c], tester[c]);
						max[c] = std::fmax(max[c], tester[c]);
					}
				}
			}
		}

		bbox = aabb(min, max);
	}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {

		// Transform the ray from world space to object space.

		auto origin = point3
		(
			(cos_theta * r.origin().x) - (sin_theta * r.origin().z),
			r.origin().y,
			(sin_theta * r.origin().x) + (cos_theta * r.origin().z)
		);

		auto direction = glm::vec3
		(
			(cos_theta * r.direction().x) - (sin_theta * r.direction().z),
			r.direction().y,
			(sin_theta * r.direction().x) + (cos_theta * r.direction().z)
		);

		ray rotated_r(origin, direction, r.time());

		// Determine whether an intersection exists in object space (and if so, where).

		if (!object->hit(rotated_r, ray_t, rec))
			return false;

		// Transform the intersection from object space back to world space.

		rec.p = point3
		(
			(cos_theta * rec.p.x) + (sin_theta * rec.p.z),
			rec.p.y,
			(-sin_theta * rec.p.x) + (cos_theta * rec.p.z)
		);

		rec.normal = glm::vec3
		(
			(cos_theta * rec.normal.x) + (sin_theta * rec.normal.z),
			rec.normal.y,
			(-sin_theta * rec.normal.x) + (cos_theta * rec.normal.z)
		);

		return true;
	}


	aabb bounding_box() const override { return bbox; }

private:
	shared_ptr<hittable> object;
	float sin_theta;
	float cos_theta;
	aabb bbox;
};

class scale : public hittable {
public:
	// Takes an object and a uniform scale factor (e.g., 2.0 to double in size, 0.5 to halve)
	scale(shared_ptr<hittable> object, float scale_factor)
		: object(object), scale_factor(scale_factor)
	{	
		aabb bbox_inner = object->bounding_box();

		point3 min_p(bbox_inner.x.min, bbox_inner.y.min, bbox_inner.z.min);
		point3 max_p(bbox_inner.x.max, bbox_inner.y.max, bbox_inner.z.max);

		bbox = aabb(min_p * scale_factor, max_p * scale_factor);
	}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
		// transform to object space by scaling the ray's origin and direction by the inverse of the scale factor
		ray scaled_r(r.origin() / scale_factor, r.direction() / scale_factor, r.time());

		// Determine whether an intersection exists in scaled object space
		if (!object->hit(scaled_r, ray_t, rec))
			return false;

		// Transform the intersection point back to world space
		rec.p *= scale_factor;

		// Note: For uniform scaling, the normal's direction does not change, 
		// so we don't need to do any math on rec.normal.

		return true;
	}

	float pdf_value(const point3& origin, const glm::vec3& direction) const override {
		return object->pdf_value(origin / scale_factor, direction);
	}

	glm::vec3 random(const point3& origin) const override {
		return object->random(origin / scale_factor) * scale_factor;
	}

	aabb bounding_box() const override { return bbox; }

private:
	shared_ptr<hittable> object;
	float scale_factor;
	aabb bbox;
};

inline shared_ptr<hittable> scale_to_height(shared_ptr<hittable> object, float target_height)
{
	aabb bbox = object->bounding_box();
	float current_height = bbox.y.size();

	// Safety check to prevent dividing by zero if the object is perfectly flat
	if (current_height < 0.00001f) {
		return object;
	}

	// The math: if it's 2 units tall and you want 10, 10 / 2 = scale factor of 5
	float scale_factor = target_height / current_height;

	return make_shared<scale>(object, scale_factor);
}

#endif // !HITTABLE_H
