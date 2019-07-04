#include "common.h"
#include "glm.h"

#include "tol.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map>

Mesh loadModel(const char* modelPath)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning, error;

	bool loadOk = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, modelPath);

	if (!loadOk)
	{
		fprintf(stderr, "WARNING: %s.\nERROR: %s.\n", warning.c_str(), error.c_str());
		assert(!"Error loading the model!");
	}

	Mesh mesh = {};
	std::unordered_map<Vertex, u32> uniqueVertices;

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex;

			vertex.pos =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0)
			{

				uniqueVertices.emplace(vertex, u32(mesh.vertices.size()));
				mesh.vertices.push_back(vertex);
			}

			mesh.indices.push_back(uniqueVertices[vertex]);
		}
	}

	return mesh;
}