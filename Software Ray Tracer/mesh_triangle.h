#ifndef MESH_TRIANGLE_H
#define MESH_TRIANGLE_H

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoords;
};

class mesh_triangle : public hittable
{
public:
	mesh_triangle(Vertex v0, Vertex v1, Vertex v2, shared_ptr<material> mat)
		: v0(v0), v1(v1), v2(v2), mat(mat)
	{
		aabb box1(v0.position, v1.position);
		aabb box2(v0.position, v2.position);
		aabb box3(v1.position, v2.position);

		bbox = aabb(aabb(box1, box2), box3);

		area = 0.5f * glm::length(glm::cross(v1.position - v0.position, v2.position - v0.position));
	}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {

		glm::vec3 edge1 = v1.position - v0.position;
		glm::vec3 edge2 = v2.position - v0.position;

		glm::vec3 h = glm::cross(r.direction(), edge2);
		float a = glm::dot(edge1, h);

		const float EPSILON = 0.0000001f;
		if (a > -EPSILON && a < EPSILON) {
			return false;
		}

		float f = 1.0f / a;

		glm::vec3 s = r.origin() - v0.position;


		float u = f * glm::dot(s, h);
		if (u < 0.0f || u > 1.0f) 
			return false;
		

		// 6. Prepare to test 'v' parameter
		glm::vec3 q = glm::cross(s, edge1);

		float v = f * glm::dot(r.direction(), q);
		if (v < 0.0f || u + v > 1.0f)
			return false;
		

		// 8. At this stage, we can compute 't' to find out where the intersection point is on the line
		float t = f * glm::dot(edge2, q);

		// 9. Check if 't' is within our acceptable ray depth bounds
		if (!ray_t.contains(t)) {
			return false;
		}

		// --- INTERSECTION CONFIRMED ---

		rec.t = t;
		rec.p = r.at(t);
		rec.mat = mat;

		float w = 1.0f - u - v;

		glm::vec2 interpolated_uv = (w * v0.texcoords) +
			(u * v1.texcoords) +
			(v * v2.texcoords);
		rec.u = interpolated_uv.x;
		rec.v = interpolated_uv.y;

		glm::vec3 interpolated_normal = (w * v0.normal) +
			(u * v1.normal) +
			(v * v2.normal);

		interpolated_normal = glm::normalize(interpolated_normal);
		rec.set_face_normal(r, interpolated_normal);

		return true;
	}

	aabb bounding_box() const override { return bbox; }

	float get_area() const override {
		return area;
	}

	glm::vec3 random(const point3& origin) const override 
	{
		// Sample a random point on the triangle using barycentric coordinates
		float r1 = random_float();
		float r2 = random_float();
		
		if (r1 + r2 > 1.0f) {
			r1 = 1.0f - r1;
			r2 = 1.0f - r2;
		}
		glm::vec3 random_point = v0.position + r1 * (v1.position - v0.position) + r2 * (v2.position - v0.position);
		return random_point - origin;
	}

private:
	Vertex v0, v1, v2;
	shared_ptr<material> mat;
	aabb bbox;
	float area;
};

#endif // MESH_TRIANGLE_H