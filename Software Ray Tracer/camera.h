#ifndef CAMERA_H
#define CAMERA_H

#include <thread>
#include <atomic>
#include <fstream>

#include "hittable.h"
#include "pdf.h"
#include "material.h"

using namespace std;

class camera 
{
public:
	float aspect_ratio = 1.0f;	   // Ratio of image width over height
	int    image_width = 100;	   // Rendered image width in pixel count
	int	   samples_per_pixel = 10; // Number of rays to cast per pixel for anti-aliasing
	int    max_depth = 10;		   // Maximum number of ray bounces into scene
	int thread_count = 24;		   // How many threads should be used to render the image

	std::ofstream image_file = std::ofstream("Renders/image.ppm");		// Output file stream for rendered image

	color  background;               // Scene background color

	float vfov = 90;				    // Vertical field of view in degrees
	point3 lookfrom = point3(0, 0, 0);  // Point camera is looking from
	point3 lookat = point3(0, 0, -1);   // Point camera is looking at
	glm::vec3   vup = glm::vec3(0,1,0); // Camera-relative "up" direction

	float defocus_angle = 0.0f;  // Variation angle of rays through each pixel
	float focus_dist = 10.0f;    // Distance from camera lookfrom point to plane of perfect focus

	void render(const hittable& world, const hittable& lights) 
	{
		initialize();

		image_file << "P3\n" << image_width << ' ' << image_height << "\n255\n";

		next_scanline = 0;
		std::vector<std::thread> threads;

		for (int i = 0; i < thread_count; i++)
		{
			threads.emplace_back(&camera::render_lines, this, std::ref(world), std::ref(lights));
		}
		
		
		for (std::thread& t : threads)
			if (t.joinable())
				t.join();

		std::clog << "\r Writing image to ppm " << std::flush;
		for (int i = 0; i < image_height; i++)
			for (int j = 0; j < image_width; j++)
				write_color(image_file, pixel_samples_scale * pixel_colors[(i * image_width) + j]);

		std::clog << "\rDone.                 \n";
	}

private:
	/* Private Camera Variables Here */
	int       image_height;			// Rendered image height
	float     pixel_samples_scale;	// Color scale factor for a sum of pixel samples
	int		  sqrt_spp;             // Square root of number of samples per pixel
	float	  recip_sqrt_spp;       // 1 / sqrt_spp
	point3    center;				// Camera center
	point3	  pixel00_loc;			// Location of pixel 0, 0
	glm::vec3 pixel_delta_u;		// Offset to pixel to the right
	glm::vec3 pixel_delta_v;		// Offset to pixel below
	glm::vec3 u, v, w;				// Camera frame basis vectors
	glm::vec3 defocus_disk_u;		// Defocus disk horizontal radius
	glm::vec3 defocus_disk_v;		// Defocus disk vertical radius

	

	atomic<int> next_scanline;	// The next scanline to be rendered by a thread. Atomically incremented by each thread as they render lines.
	vector<color> pixel_colors; // Accumulated color samples for each pixel. Indexed as (row * image_width) + column.

	void initialize() 
	{
		image_height = int(image_width / aspect_ratio);
		image_height = (image_height < 1) ? 1 : image_height;

		sqrt_spp = int(std::sqrt(samples_per_pixel));
		pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp);
		recip_sqrt_spp = 1.0 / sqrt_spp;

		pixel_colors.resize(image_height * image_width);

		center = lookfrom;

		// Determine viewport dimensions.
		auto theta = degrees_to_radians(vfov);
		auto h = std::tan(theta / 2);
		auto viewport_height = 2 * h * focus_dist;
		auto viewport_width = viewport_height * (float(image_width) / image_height);

		// Calculate the u,v,w unit basis vectors for the camera coordinate frame.
		w = glm::normalize(lookfrom - lookat);
		u = glm::normalize(cross(vup, w));
		v = cross(w, u);

		// Calculate the vectors across the horizontal and down the vertical viewport edges.
		glm::vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
		glm::vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge


		// Calculate the horizontal and vertical delta vectors from pixel to pixel.
		pixel_delta_u = viewport_u / float(image_width);
		pixel_delta_v = viewport_v / float(image_height);

		// Calculate the location of the upper left pixel.
		auto viewport_upper_left = center - (focus_dist * w) - viewport_u / 2.0f - viewport_v / 2.0f;
		pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);

		// Calculate the camera defocus disk basis vectors.
		auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
		defocus_disk_u = u * defocus_radius;
		defocus_disk_v = v * defocus_radius;
	}

	void render_lines(const hittable& world, const hittable& lights)
	{
		int current_line = 0;
		while ((current_line = next_scanline++) < image_height) {
			std::clog << "\rScanlines remaining: " << image_height - current_line << ' ' << std::flush;

			for (int i = 0; i < image_width; i++) {
				color pixel_color(0, 0, 0);
				
				for (int s_j = 0; s_j < sqrt_spp; s_j++) {
					for (int s_i = 0; s_i < sqrt_spp; s_i++) {
						ray r = get_ray(i, current_line, s_i, s_j);
						pixel_color += ray_color(r, max_depth, world,lights);
					}
				}
			
				pixel_colors[(current_line * image_width) + i] = pixel_color;
			}
		}
	}

	ray get_ray(int i, int j, int s_i, int s_j) const {
		// Construct a camera ray originating from the defocus disk and directed at a randomly
		// sampled point around the pixel location i, j.

		auto offset = sample_square_stratified(s_i, s_j);
		auto pixel_sample = pixel00_loc
						  + ((i + offset.x) * pixel_delta_u)
						  + ((j + offset.y) * pixel_delta_v);

		auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
		auto ray_direction = pixel_sample - ray_origin;
		auto ray_time = random_float();

		return ray(ray_origin, ray_direction, ray_time);
	}

	glm::vec3 sample_square_stratified(int s_i, int s_j) const {
		// Returns the vector to a random point in the square sub-pixel specified by grid
		// indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].

		auto px = ((s_i + random_float()) * recip_sqrt_spp) - 0.5f;
		auto py = ((s_j + random_float()) * recip_sqrt_spp) - 0.5f;

		return glm::vec3(px, py, 0);
	}

	point3 defocus_disk_sample() const {
		// Returns a random point on the camera's defocus disk, which is centered at the camera origin and oriented in the plane of the viewport.
		auto disk_sample = random_in_unit_disk();
		return center + (disk_sample.x * defocus_disk_u) + (disk_sample.y * defocus_disk_v);
	}

	glm::vec3 sample_square() const {
		// Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
		return glm::vec3(random_float() - 0.5f, random_float() - 0.5f, 0);
	}

	color ray_color(const ray& r, int depth, const hittable& world, const hittable& lights) const {
		if (depth <= 0)
			return color(0, 0, 0);

		hit_record rec;
		if (!world.hit(r, interval(0.001f, infinity), rec))
			return background;
		
		scatter_record srec;
		color color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

		if (!rec.mat->scatter(r, rec, srec))
			return color_from_emission;

		if (srec.skip_pdf) 
			return srec.attenuation * ray_color(srec.skip_pdf_ray, depth - 1, world, lights);

		auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
		mixture_pdf p(light_ptr, srec.pdf_ptr);

		ray scattered = ray(rec.p, p.generate(), r.time());
		auto pdf_value = p.value(scattered.direction());

		float scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);

		color sample_color = ray_color(scattered, depth - 1, world, lights);
		color color_from_scatter =
			(srec.attenuation * scattering_pdf * sample_color) / pdf_value;

		return color_from_emission + color_from_scatter;
	
	}
};

#endif