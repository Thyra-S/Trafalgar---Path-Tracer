#ifndef PDF_H
#define PDF_H

#include "onb.h"


class pdf {
public:
	virtual ~pdf() {}

	virtual float value(const glm::vec3& direction) const = 0;
	virtual glm::vec3 generate() const = 0;
};


class sphere_pdf : public pdf {
public:
	sphere_pdf() {}

	float value(const glm::vec3& direction) const override 
	{
		return 1 / (4 * pi);
	}

	glm::vec3 generate() const override 
	{
		return random_unit_vector();
	}
};

class cosine_pdf : public pdf
{
public:
	cosine_pdf(const glm::vec3& w) : uvw(w) {}

	float value(const glm::vec3& direction) const override
	{
		auto cosine = dot(normalize(direction), uvw.w());
		return cosine <= 0 ? 0 : (cosine / pi);
	}
	glm::vec3 generate() const override
	{
		return uvw.transform(random_cosine_direction());
	}
private:
	onb uvw;
};

class hittable_pdf : public pdf
{
public:
	hittable_pdf(const hittable& objects, const point3& origin) : origin(origin), objects(objects) {}

	float value(const glm::vec3& direction) const override
	{
		return objects.pdf_value(origin, direction);
	}

	glm::vec3 generate() const override
	{
		return objects.random(origin);
	}
private:
	const hittable& objects;
	point3 origin;
};

class mixture_pdf : public pdf {
public:
	mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) {
		p[0] = p0;
		p[1] = p1;
	}

	float value(const glm::vec3& direction) const override {
		return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
	}

	glm::vec3 generate() const override {
		if (random_float() < 0.5)
			return p[0]->generate();
		else
			return p[1]->generate();
	}

private:
	shared_ptr<pdf> p[2];
};

#endif