#ifndef MODEL_H
#define MODEL_H

#include "external/tiny_obj_loader.h"
#include "mesh.h"
#include "mesh_triangle.h"

class model
{
public:
	model(const char* filename, shared_ptr<material> mat)
	{
		load_model(filename, mat);
	}

	// Loads a model from an .obj file and adds it to the scene as a collection of triangles with the given material.
	void load_model(const char* filename, shared_ptr<material> mat)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename)) {
			throw std::runtime_error(warn + err);
		}

		// Don't need to de duplicate for software ray tracer,
		// std::unordered_map<glm::vec3, uint32_t> uniqueVertices;

		std::vector<Vertex> vertices;
		std::vector<size_t> indices;
		hittable_list objects;

		for (const auto& shape : shapes) {
			size_t index_offset = 0;

			// Loop over every face (polygon) in the shape
			for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
				size_t fv = size_t(shape.mesh.num_face_vertices[f]);

				// only triangles
				if (fv != 3) {
					index_offset += fv;
					continue;
				}

				Vertex triangle_verts[3];

				// loop over vertices in the face
				for (size_t v = 0; v < 3; v++) 
				{
					tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

					// extract position
					triangle_verts[v].position = glm::vec3(
						attrib.vertices[3 * idx.vertex_index + 0],
						attrib.vertices[3 * idx.vertex_index + 1],
						attrib.vertices[3 * idx.vertex_index + 2]
					);

					// extract normals
					if (idx.normal_index >= 0) 
					{
						triangle_verts[v].normal = glm::normalize(glm::vec3(
							attrib.normals[3 * idx.normal_index + 0],
							attrib.normals[3 * idx.normal_index + 1],
							attrib.normals[3 * idx.normal_index + 2]
						));
					}
					else 
						// if no normal data is provided, we will calculate it later using the cross 
						// product of the triangle edges. For now, we can initialize it to zero.
						triangle_verts[v].normal = glm::vec3(0.0f);

					// extract texture coordinates
					if (idx.texcoord_index >= 0) 
					{
						triangle_verts[v].texcoords = glm::vec2(
							attrib.texcoords[2 * idx.texcoord_index + 0],
							attrib.texcoords[2 * idx.texcoord_index + 1]
						);
					}
					else 
						triangle_verts[v].texcoords = glm::vec2(0.0f);
				}
				index_offset += fv;

				// calculate missing normals
				if (triangle_verts[0].normal == glm::vec3(0.0f)) {
					glm::vec3 edge1 = triangle_verts[1].position - triangle_verts[0].position;
					glm::vec3 edge2 = triangle_verts[2].position - triangle_verts[0].position;
					glm::vec3 flat_normal = glm::normalize(glm::cross(edge1, edge2));

					triangle_verts[0].normal = flat_normal;
					triangle_verts[1].normal = flat_normal;
					triangle_verts[2].normal = flat_normal;
				}

				// create the mesh triangle and add it to the list of objects with the inputted material
				// might want to consider using the material from the .mtl file if available, but for now we will use the provided material
				objects.add(make_shared<mesh_triangle>(
					triangle_verts[0],
					triangle_verts[1],
					triangle_verts[2],
					mat
				));
			}
		}

		m = mesh(objects);
	}

	shared_ptr<hittable> get_mesh() 
	{
		return std::make_shared<mesh>(m);
	}

private:
	
	mesh m;
};

#endif