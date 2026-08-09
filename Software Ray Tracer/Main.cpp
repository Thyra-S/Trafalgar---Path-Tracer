#include "rtweekend_commons.h"
#include "bvh.h"
#include "camera.h"
#include "constant_medium.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "quad.h"
#include "triangle.h"
#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#define TINYOBJLOADER_IMPLEMENTATION
#include "model.h"
#include "mesh.h"
#include "texture.h"


#define THREAD_COUNT 8

struct camera_parameters
{
	float aspect_ratio;
	int image_width;
	int samples_per_pixel;
	int max_depth;
	int thread_count;
	color background;
	float vfov;
	point3 lookfrom;
	point3 lookat;
	glm::vec3 vup;
	float defocus_angle;
	float focus_dist;
};

hittable_list world;
quad lights;
camera_parameters params;

void bouncing_spheres()
{
	auto checker = make_shared<checker_texture>(0.32f, color(0.2f, 0.3f, 0.1f), color(0.9f, 0.9f, 0.9f));
	world.add(make_shared<sphere>(point3(0.0f, -1000.0f, 0.0f), 1000.0f, make_shared<lambertian>(checker)));

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto choose_mat = random_float();
			point3 center(a + 0.9f * random_float(), 0.2f, b + 0.9f * random_float());

			if ((center - point3(4, 0.2f, 0)).length() > 0.9f) {
				shared_ptr<material> sphere_material;

				if (choose_mat < 0.8f) {
					// diffuse
					auto albedo = glm::vec3(random_float(), random_float(), random_float()) 
								* glm::vec3(random_float(),random_float(),random_float());
					sphere_material = make_shared<lambertian>(albedo);
					auto center2 = center + glm::vec3(0, random_float(0,0.5f), 0);
					world.add(make_shared<sphere>(center,center2, 0.2f, sphere_material));
				}
				else if (choose_mat < 0.95f) {
					// metal
					auto albedo = color(random_float(0.5f, 1.0f), random_float(0.5f, 1.0f), random_float(0.5f, 1.0f));
					auto fuzz = random_float(0.0f, 0.5f);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(make_shared<sphere>(center, 0.2f, sphere_material));
				}
				else {
					// glass
					sphere_material = make_shared<dielectric>(1.5f);
					world.add(make_shared<sphere>(center, 0.2f, sphere_material));
				}
			}
		}
	}

	auto material1 = make_shared<dielectric>(1.5f);
	world.add(make_shared<sphere>(point3(0.0f, 1.0f, 0.0f), 1.0f, material1));

	auto material2 = make_shared<lambertian>(color(0.4f, 0.2f, 0.1f));
	world.add(make_shared<sphere>(point3(-4.0f, 1.0f, 0.0f), 1.0f, material2));

	auto material3 = make_shared<metal>(color(0.7f, 0.6f, 0.5f), 0.0f);
	world.add(make_shared<sphere>(point3(4.0f, 1.0f, 0.0f), 1.0f, material3));

	world = hittable_list(make_shared<bvh_node>(world));

	params =
	{
		.aspect_ratio = 16.0f / 9.0f,
		.image_width = 960,
		.samples_per_pixel = 64,
		.max_depth = 50,
		.background = color(0.70f, 0.80f, 1.00f),
		.vfov = 20.0f,
		.lookfrom = point3(13.0f, 2.0f, 3.0f),
		.lookat = point3(0.0f, 0.0f, 0.0f),
		.vup = glm::vec3(0.0f, 1.0f, 0.0f),
		.defocus_angle = 0.6f,
		.focus_dist = 10.0f
	};
}

void checkered_spheres() 
{
	auto checker = make_shared<checker_texture>(0.03f, color(0.2f, 0.3f, 0.1f), color(0.9f, 0.9f, 0.9f));

	world.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
	world.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

	params =
	{
		.aspect_ratio = 16.0f / 9.0f,
		.image_width = 400,
		.samples_per_pixel = 100,
		.max_depth = 50,
		.background = color(0.70f, 0.80f, 1.00f),
		.vfov = 20.0f,
		.lookfrom = point3(13.0f, 2.0f, 3.0f),
		.lookat = point3(0.0f, 0.0f, 0.0f),
		.vup = glm::vec3(0.0f, 1.0f, 0.0f),
		.defocus_angle = 0.0f,
		.focus_dist = 10.0f
	};
}

void earth()
{
	auto earth_texture = make_shared<image_texture>("images/earthmap.jpg");
	auto earth_surface = make_shared<lambertian>(earth_texture);
	auto globe = make_shared<sphere>(point3(0.0f, 0.0f, 0.0f), 2.0f, earth_surface);
	camera cam;

	params =
	{
		.aspect_ratio = 16.0f / 9.0f,
		.image_width = 400,
		.samples_per_pixel = 100,
		.max_depth = 50,
		.background = color(0.70f, 0.80f, 1.00f),
		.vfov = 20.0f,
		.lookfrom = point3(0.0f, 0.0f, 12.0f),
		.lookat = point3(0.0f, 0.0f, 0.0f),
		.vup = glm::vec3(0.0f, 1.0f, 0.0f),
		.defocus_angle = 0.0f,
		.focus_dist = 10.0f
	};
}

void perlin_spheres() 
{
	hittable_list world;

	auto pertext = make_shared<noise_texture>(4.0f);
	world.add(make_shared<sphere>(point3(0.0f, -1000.0f, 0.0f), 1000.0f, make_shared<lambertian>(pertext)));
	world.add(make_shared<sphere>(point3(0.0f, 2.0f, 0.0f), 2.0f, make_shared<lambertian>(pertext)));

	params =
	{
		.aspect_ratio = 16.0f / 9.0f,
		.image_width = 400,
		.samples_per_pixel = 100,
		.max_depth = 50,
		.background = color(0.70f, 0.80f, 1.00f),
		.vfov = 20.0f,
		.lookfrom = point3(13.0f, 2.0f, 3.0f),
		.lookat = point3(0.0f, 0.0f, 0.0f),
		.vup = glm::vec3(0.0f, 1.0f, 0.0f),
		.defocus_angle = 0.0f,
		.focus_dist = 10.0f
	};
}

void quads() 
{
	// Materials
	auto left_red = make_shared<lambertian>(color(1.0f, 0.2f, 0.2f));
	auto back_green = make_shared<lambertian>(color(0.2f, 1.0f, 0.2f));
	auto right_blue = make_shared<lambertian>(color(0.2f, 0.2f, 1.0f));
	auto upper_orange = make_shared<lambertian>(color(1.0f, 0.5f, 0.0f));
	auto lower_teal = make_shared<lambertian>(color(0.2f, 0.8f, 0.8f));

	// Quads
	world.add(make_shared<quad>(point3(-3.0f, -2.0f, 5.0f), glm::vec3(0.0f, 0.0f, -4.0f), glm::vec3(0.0f, 4.0f, 0.0f), left_red));
	world.add(make_shared<quad>(point3(-2.0f, -2.0f, 0.0f), glm::vec3(4.0f, 0.0f, 0.0f), glm::vec3(0.0f, 4.0f, 0.0f), back_green));
	world.add(make_shared<quad>(point3(3.0f, -2.0f, 1.0f), glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 4.0f, 0.0f), right_blue));
	world.add(make_shared<quad>(point3(-2.0f, 3.0f, 1.0f), glm::vec3(4.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 4.0f), upper_orange));
	world.add(make_shared<quad>(point3(-2.0f, -3.0f, 5.0f), glm::vec3(4.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -4.0f), lower_teal));

	params =
	{
		.aspect_ratio = 1.0f,
		.image_width = 400,
		.samples_per_pixel = 100,
		.max_depth = 50,
		.background = color(0.70f, 0.80f, 1.00f),
		.vfov = 80.0f,
		.lookfrom = point3(0.0f, 0.0f, 9.0f),
		.lookat = point3(0.0f, 0.0f, 0.0f),
		.vup = glm::vec3(0.0f, 1.0f, 0.0f),
		.defocus_angle = 0.0f,
		.focus_dist = 10.0f
	};
}

void tris() 
{
	// Materials
	auto left_red = make_shared<lambertian>(color(1.0f, 0.2f, 0.2f));
	auto back_green = make_shared<lambertian>(color(0.2f, 1.0f, 0.2f));
	auto right_blue = make_shared<lambertian>(color(0.2f, 0.2f, 1.0f));
	auto upper_orange = make_shared<lambertian>(color(1.0f, 0.5f, 0.0f));
	auto lower_teal = make_shared<lambertian>(color(0.2f, 0.8f, 0.8f));

	// Quads made of triangles
	world.add(make_shared<triangle>(point3(-3.0f, -2.0f, 5.0f), glm::vec3(0.0f, 0.0f, -4.0f), glm::vec3(0.0f, 4.0f, 0.0f), left_red));
	world.add(make_shared<triangle>(point3(-3.0f, 2.0f, 1.0f), glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, -4.0f, 0.0f), right_blue));

	world.add(make_shared<triangle>(point3(-2.0f, -2.0f, 0.0f), glm::vec3(4.0f, 0.0f, 0.0f), glm::vec3(0.0f, 4.0f, 0.0f), back_green));
	world.add(make_shared<triangle>(point3(2.0f, 2.0f, 0.0f), glm::vec3(-4.0f, 0.0f, 0.0f), glm::vec3(0.0f, -4.0f, 0.0f), upper_orange));

	world.add(make_shared<triangle>(point3(3.0f, -2.0f, 1.0f), glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 4.0f, 0.0f), right_blue));
	world.add(make_shared<triangle>(point3(3.0f, 2.0f, 5.0f), glm::vec3(0.0f, 0.0f, -4.0f), glm::vec3(0.0f, -4.0f, 0.0f), left_red));

	world.add(make_shared<triangle>(point3(-2.0f, 3.0f, 1.0f), glm::vec3(4.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 4.0f), upper_orange));
	world.add(make_shared<triangle>(point3(2.0f, 3.0f, 5.0f), glm::vec3(-4.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -4.0f), lower_teal));

	world.add(make_shared<triangle>(point3(-2.0f, -3.0f, 5.0f), glm::vec3(4.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -4.0f), lower_teal));
	world.add(make_shared<triangle>(point3(2.0f, -3.0f, 1.0f), glm::vec3(-4.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 4.0f), back_green));

	params =
	{
		.aspect_ratio = 1.0f,
		.image_width = 400,
		.samples_per_pixel = 100,
		.max_depth = 50,
		.background = color(0.70f, 0.80f, 1.00f),
		.vfov = 80.0f,
		.lookfrom = point3(0.0f, 0.0f, 9.0f),
		.lookat = point3(0.0f, 0.0f, 0.0f),
		.vup = glm::vec3(0.0f, 1.0f, 0.0f),
		.defocus_angle = 0.0f,
		.focus_dist = 10.0f
	};
}

void simple_light() {

	auto pertext = make_shared<noise_texture>(4);
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
	world.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

	auto difflight = make_shared<diffuse_light>(color(4, 4, 4));
	world.add(make_shared<quad>(point3(3, 1, -2), glm::vec3(2, 0, 0), glm::vec3(0, 2, 0), difflight));
	params =
	{
		.aspect_ratio = 16.0f / 9.0f,
		.image_width = 400,
		.samples_per_pixel = 100,
		.max_depth = 50,
		.background = color(0, 0, 0),
		.vfov = 20,
		.lookfrom = point3(26, 3, 6),
		.lookat = point3(0, 2, 0),
		.vup = glm::vec3(0, 1, 0),
		.defocus_angle = 0,
		.focus_dist = 10
	};
}

void cornell_box()
{
	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto blue = make_shared<lambertian>(color(.12, .15, .45));
	auto light = make_shared<diffuse_light>(color(15, 15, 15));

	world.add(make_shared<quad>(point3(555, 0, 0), glm::vec3(0, 555, 0), glm::vec3(0, 0, 555), green));
	world.add(make_shared<quad>(point3(0, 0, 0), glm::vec3(0, 555, 0), glm::vec3(0, 0, 555), red));
	world.add(make_shared<quad>(point3(343, 554, 332), glm::vec3(-130, 0, 0), glm::vec3(0, 0, -105), light));
	world.add(make_shared<quad>(point3(0, 0, 0), glm::vec3(555, 0, 0), glm::vec3(0, 0, 555), white));
	world.add(make_shared<quad>(point3(555, 555, 555), glm::vec3(-555, 0, 0), glm::vec3(0, 0, -555), white));
	world.add(make_shared<quad>(point3(0, 0, 555), glm::vec3(555, 0, 0), glm::vec3(0, 555, 0), white));

	// edge length of the tetrahedron
	double s = 200;

	// Calculate the vertices for a perfectly equilateral tetrahedron
	point3 A(0, 0, 0);
	point3 B(0, 0, s);
	point3 C(s * std::sqrt(3.0) / 2.0, 0, s / 2.0);

	// The peak sits above the centroid of the base
	double centroid_x = s * std::sqrt(3.0) / 6.0;
	double centroid_z = s / 2.0;
	double height = s * std::sqrt(2.0 / 3.0);
	point3 D(centroid_x, height, centroid_z);

	// Create the tetrahedron
	auto glass = make_shared<dielectric>(1.5f);
	shared_ptr<hittable> tet1 = tetrahedron(A, B, C, D, glass);

	//shared_ptr<hittable> box1 = box(point3(0, 0, 0), point3(165, 330, 165), white);
	tet1 = make_shared<rotate_y>(tet1, 75);
	tet1 = make_shared<translate>(tet1, glm::vec3(300, 20, 300));

	world.add(tet1);

	shared_ptr<hittable> box2 = box(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, glm::vec3(130, 0, 65));
	world.add(box2);

	params =
	{
		.aspect_ratio = 1.0f,
		.image_width = 600,
		.samples_per_pixel = 1000,
		.max_depth = 50,
		.background = color(0, 0, 0),
		.vfov = 40.0f,
		.lookfrom = point3(278, 278, -800),
		.lookat = point3(278, 278, 0),
		.vup = glm::vec3(0, 1, 0),
		.defocus_angle = 0.0f,
		.focus_dist = 10.0f
	};
}


void cornell_smoke() {
	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto light = make_shared<diffuse_light>(color(7, 7, 7));

	world.add(make_shared<quad>(point3(555, 0, 0), glm::vec3(0, 555, 0), glm::vec3(0, 0, 555), green));
	world.add(make_shared<quad>(point3(0, 0, 0), glm::vec3(0, 555, 0), glm::vec3(0, 0, 555), red));
	world.add(make_shared<quad>(point3(113, 554, 127), glm::vec3(330, 0, 0), glm::vec3(0, 0, 305), light));
	world.add(make_shared<quad>(point3(0, 555, 0), glm::vec3(555, 0, 0), glm::vec3(0, 0, 555), white));
	world.add(make_shared<quad>(point3(0, 0, 0), glm::vec3(555, 0, 0), glm::vec3(0, 0, 555), white));
	world.add(make_shared<quad>(point3(0, 0, 555), glm::vec3(555, 0, 0), glm::vec3(0, 555, 0), white));

	shared_ptr<hittable> box1 = box(point3(0, 0, 0), point3(165, 330, 165), white);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, glm::vec3(265, 0, 295));

	shared_ptr<hittable> box2 = box(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, glm::vec3(130, 0, 65));

	world.add(make_shared<constant_medium>(box1, 0.01f, color(0, 0, 0)));
	world.add(make_shared<constant_medium>(box2, 0.01f, color(1, 1, 1)));

	params =
	{
		.aspect_ratio = 1.0f,
		.image_width = 600,
		.samples_per_pixel = 200,
		.max_depth = 50,
		.background = color(0, 0, 0),
		.vfov = 40.0f,
		.lookfrom = point3(278, 278, -800),
		.lookat = point3(278, 278, 0),
		.vup = glm::vec3(0, 1, 0),
		.defocus_angle = 0.0f,
		.focus_dist = 10.0f
	};
}

void final_scene() {
	hittable_list boxes1;
	auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

	int boxes_per_side = 20;
	for (int i = 0; i < boxes_per_side; i++) {
		for (int j = 0; j < boxes_per_side; j++) {
			auto w = 100.0;
			auto x0 = -1000.0 + i * w;
			auto z0 = -1000.0 + j * w;
			auto y0 = 0.0;
			auto x1 = x0 + w;
			auto y1 = random_float(1, 101);
			auto z1 = z0 + w;

			boxes1.add(box(point3(x0, y0, z0), point3(x1, y1, z1), ground));
		}
	}
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	world.add(make_shared<bvh_node>(boxes1));

	auto light = make_shared<diffuse_light>(color(7, 7, 7));
	world.add(make_shared<quad>(point3(123, 554, 147), glm::vec3(300, 0, 0), glm::vec3(0, 0, 265), light));

	auto center1 = point3(400, 400, 200);
	auto center2 = center1 + glm::vec3(30, 0, 0);
	auto sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
	world.add(make_shared<sphere>(center1, center2, 50, sphere_material));

	// world.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
	world.add(make_shared<sphere>(point3(60, 180, 200), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)));

	auto boundary = make_shared<sphere>(point3(360, 150, 145), 70, make_shared<dielectric>(1.5));
	world.add(boundary);
	world.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
	boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
	world.add(make_shared<constant_medium>(boundary, .0001, color(1, 1, 1)));

	// Dielectric tetrahedron at center of scene
	auto tet = equilateral_tetrahedron(360, make_shared<dielectric>(1.5));
	tet = make_shared<rotate_y>(tet, 90);
	tet = make_shared<translate>(tet, glm::vec3(-80, 100, 100));

	world.add(tet);

	auto emat = make_shared<lambertian>(make_shared<image_texture>("earthmap.jpg"));
	world.add(make_shared<sphere>(point3(400, 200, 400), 100, emat));
	auto pertext = make_shared<noise_texture>(0.02f, 0.01f);
	world.add(make_shared<sphere>(point3(220, 280, 300), 80, make_shared<lambertian>(pertext)));

	hittable_list boxes2;
	int ns = 1000;
	for (int j = 0; j < ns; j++) {
		boxes2.add(make_shared<sphere>(point3(random_float(0, 165), random_float(0, 165), random_float(0, 165)), 10, white));
	}

	world.add(make_shared<translate> (
		make_shared<rotate_y>(make_shared<bvh_node>(boxes2), 15),
		glm::vec3(-100, 270, 395)
	)
	);

	auto glass = make_shared<dielectric>(1.5f);

	hittable_list teapot_geom;
	model teapot_model("models/utah_teapot.obj", glass);

	shared_ptr<hittable> teapot_mesh = teapot_model.get_mesh();

	teapot_mesh = scale_to_height(teapot_mesh, 200.0f);

	teapot_mesh = make_shared<translate>(teapot_mesh, point3(277, 50, 277));

	world.add(teapot_mesh);

	world = hittable_list(make_shared<bvh_node>(world));

	params =
	{
		.aspect_ratio = 16.0f/10.0f,
		.image_width = 1000,
		.samples_per_pixel = 4096,
		.max_depth = 25,
		.background = color(0, 0, 0),
		.vfov = 40.0f,
		.lookfrom = point3(478, 278, -600),
		.lookat = point3(278, 278, 0),
		.vup = glm::vec3(0, 1, 0),
		.defocus_angle = 0.0f,
		.focus_dist = 10.0f
	};
}

void cornell_teapot() {
	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto blue = make_shared<lambertian>(color(.12, .15, .45));
	auto light = make_shared<diffuse_light>(color(15, 15, 15));

	world.add(make_shared<quad>(point3(555, 0, 0), glm::vec3(0, 555, 0), glm::vec3(0, 0, 555), green));
	world.add(make_shared<quad>(point3(0, 0, 0), glm::vec3(0, 555, 0), glm::vec3(0, 0, 555), red));
	world.add(make_shared<quad>(point3(343, 554, 332), glm::vec3(-130, 0, 0), glm::vec3(0, 0, -105), light));
	world.add(make_shared<quad>(point3(0, 0, 0), glm::vec3(555, 0, 0), glm::vec3(0, 0, 555), white));
	world.add(make_shared<quad>(point3(555, 555, 555), glm::vec3(-555, 0, 0), glm::vec3(0, 0, -555), white));
	world.add(make_shared<quad>(point3(0, 0, 555), glm::vec3(555, 0, 0), glm::vec3(0, 555, 0), white));

	auto glass = make_shared<dielectric>(1.5f);

	model teapot_model("models/utah_teapot.obj", glass);

	shared_ptr<hittable> teapot_mesh = teapot_model.get_mesh();

	teapot_mesh = scale_to_height(teapot_mesh, 200.0f);

	teapot_mesh = make_shared<translate>(teapot_mesh, point3(277, 50, 277));

	world.add(teapot_mesh);

	world = hittable_list(make_shared<bvh_node>(world));

	params =
	{
		.aspect_ratio = 1.0f,
		.image_width = 1000,
		.samples_per_pixel = 4096,
		.max_depth = 25,
		.background = color(0, 0, 0),
		.vfov = 40.0f,
		.lookfrom = point3(278, 278, -800),
		.lookat = point3(278, 278, 0),
		.vup = glm::vec3(0, 1, 0),
		.defocus_angle = 0.0f,
		.focus_dist = 10.0f
	};
}

void cornell_nike() {
	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto blue = make_shared<lambertian>(color(.12, .15, .45));
	auto light = make_shared<diffuse_light>(color(15, 15, 15));

	world.add(make_shared<quad>(point3(555, 0, 0), glm::vec3(0, 555, 0), glm::vec3(0, 0, 555), green));
	world.add(make_shared<quad>(point3(0, 0, 0), glm::vec3(0, 555, 0), glm::vec3(0, 0, 555), red));
	world.add(make_shared<quad>(point3(343, 554, 332), glm::vec3(-130, 0, 0), glm::vec3(0, 0, -105), light));
	world.add(make_shared<quad>(point3(0, 0, 0), glm::vec3(555, 0, 0), glm::vec3(0, 0, 555), white));
	world.add(make_shared<quad>(point3(555, 555, 555), glm::vec3(-555, 0, 0), glm::vec3(0, 0, -555), white));
	world.add(make_shared<quad>(point3(0, 0, 555), glm::vec3(555, 0, 0), glm::vec3(0, 555, 0), white));

	auto glass = make_shared<dielectric>(1.5f);

	model nike_model("models/Winged_Victory_Of_Samothrace.obj", glass);

	shared_ptr<hittable> nike_mesh = nike_model.get_mesh();

	nike_mesh = scale_to_height(nike_mesh, 450.0f);

	nike_mesh = make_shared<translate>(nike_mesh, point3(277, 25, 277));

	world.add(nike_mesh);

	world = hittable_list(make_shared<bvh_node>(world));

	params =
	{
		.aspect_ratio = 1.0f,
		.image_width = 1000,
		.samples_per_pixel = 4096,
		.max_depth = 25,
		.background = color(0, 0, 0),
		.vfov = 40.0f,
		.lookfrom = point3(278, 278, -800),
		.lookat = point3(278, 278, 0),
		.vup = glm::vec3(0, 1, 0),
		.defocus_angle = 0.0f,
		.focus_dist = 10.0f
	};
}

void cornell_miku() {
	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto blue = make_shared<lambertian>(color(.12, .15, .45));
	auto light = make_shared<diffuse_light>(color(7, 7, 7));

	world.add(make_shared<quad>(point3(555, 0, 0), glm::vec3(0, 555, 0), glm::vec3(0, 0, 555), green));
	world.add(make_shared<quad>(point3(0, 0, 0), glm::vec3(0, 555, 0), glm::vec3(0, 0, 555), red));
	world.add(make_shared<quad>(point3(113, 554, 127), glm::vec3(330, 0, 0), glm::vec3(0, 0, 305), light));
	world.add(make_shared<quad>(point3(0, 0, 0), glm::vec3(555, 0, 0), glm::vec3(0, 0, 555), white));
	world.add(make_shared<quad>(point3(555, 555, 555), glm::vec3(-555, 0, 0), glm::vec3(0, 0, -555), white));
	world.add(make_shared<quad>(point3(0, 0, 555), glm::vec3(555, 0, 0), glm::vec3(0, 555, 0), white));

	auto glass = make_shared<dielectric>(1.5f);
	auto texture = make_shared<lambertian>(make_shared<image_texture>("models/miku/miku_texture.png"));

	model miku_model("models/miku/miku.obj", texture);

	shared_ptr<hittable> miku_mesh = miku_model.get_mesh();

	miku_mesh = scale_to_height(miku_mesh, 450.0f);

	miku_mesh = make_shared<rotate_y>(miku_mesh, 180);

	miku_mesh = make_shared<translate>(miku_mesh, point3(277, -10, 277));

	world.add(miku_mesh);

	world = hittable_list(make_shared<bvh_node>(world));

	params =
	{
		.aspect_ratio = 1.0f,
		.image_width = 1000,
		.samples_per_pixel = 4096,
		.max_depth = 25,
		.background = color(0, 0, 0),
		.vfov = 40.0f,
		.lookfrom = point3(278, 278, -800),
		.lookat = point3(278, 278, 0),
		.vup = glm::vec3(0, 1, 0),
		.defocus_angle = 0.0f,
		.focus_dist = 10.0f
	};
}

void cornell()
{
	hittable_list scene;

	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto light = make_shared<diffuse_light>(color(15, 15, 15));

	// Cornell box sides
	scene.add(make_shared<quad>(point3(555, 0, 0), glm::vec3(0, 0, 555), glm::vec3(0, 555, 0), green));
	scene.add(make_shared<quad>(point3(0, 0, 555), glm::vec3(0, 0, -555), glm::vec3(0, 555, 0), red));
	scene.add(make_shared<quad>(point3(0, 555, 0), glm::vec3(555, 0, 0), glm::vec3(0, 0, 555), white));
	scene.add(make_shared<quad>(point3(0, 0, 555), glm::vec3(555, 0, 0), glm::vec3(0, 0, -555), white));
	scene.add(make_shared<quad>(point3(555, 0, 555), glm::vec3(-555, 0, 0), glm::vec3(0, 555, 0), white));

	// Light
	scene.add(make_shared<quad>(point3(213, 554, 227), glm::vec3(130, 0, 0), glm::vec3(0, 0, 105), light));

	// Box 1
	shared_ptr<material> aluminum = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
	shared_ptr<hittable> box1 = box(point3(0, 0, 0), point3(165, 330, 165), aluminum);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, glm::vec3(265, 0, 295));
	scene.add(box1);

	// Box 2
	shared_ptr<hittable> box2 = box(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, glm::vec3(130, 0, 65));
	scene.add(box2);

	world = hittable_list(make_shared<bvh_node>(scene));

	// Light Sources
	auto empty_material = shared_ptr<material>();
	lights = quad(point3(343, 554, 332), glm::vec3(-130, 0, 0), glm::vec3(0, 0, -105), empty_material);

	params =
	{
		.aspect_ratio = 1.0f,
		.image_width = 600,
		.samples_per_pixel = 64,
		.max_depth = 50,
		.background = color(0, 0, 0),
		.vfov = 40,
		.lookfrom = point3(278, 278, -800),
		.lookat = point3(278, 278, 0),
		.vup = glm::vec3(0, 1, 0),
		.defocus_angle = 0,
		.focus_dist = 800.0f
	};
}

void set_camera_parameters(camera& cam, camera_parameters params) 
{
	cam.aspect_ratio = params.aspect_ratio;
	cam.image_width = params.image_width;
	cam.samples_per_pixel = params.samples_per_pixel;
	cam.max_depth = params.max_depth;
	cam.thread_count = params.thread_count;
	cam.background = params.background;
	cam.vfov = params.vfov;
	cam.lookfrom = params.lookfrom;
	cam.lookat = params.lookat;
	cam.vup = params.vup;
	cam.defocus_angle = params.defocus_angle;
	cam.focus_dist = params.focus_dist;
}

int main()
{
	switch (14)
	{
	case 1: bouncing_spheres(); break;
	case 2: checkered_spheres(); break;
	case 3: earth(); break;
	case 4: perlin_spheres(); break;
	case 5: quads(); break;
	case 6: tris(); break;
	case 7: simple_light(); break;
	case 8: cornell_box(); break;
	case 9: cornell_smoke(); break;
	case 10: final_scene(); break;
	case 11: cornell_teapot(); break;
	case 12: cornell_nike(); break;
	case 13: cornell_miku(); break;
	case 14: cornell(); break;
	}

	camera cam;

	// performance impacting params, comment to use defaults for scenes
	// params.image_width = 1000;
	params.samples_per_pixel = 1000;
	// params.max_depth = 25;
	params.thread_count = 8;

	set_camera_parameters(cam, params);

	cam.render(world,lights);
}