#include "common.h"
#include "glm.h"

#include "model.h"

#include <meshoptimizer.h>
#include <fast_obj.h>

Mesh loadMesh_fast(const char* path)
{
	fastObjMesh* obj = fast_obj_read(path);

	size_t totalIndices = 0;

	for (u32 i = 0; i < obj->face_count; i++)
	{
		totalIndices += 3 * (obj->face_vertices[i] - 2);
	}

	vector<Vertex> vertices(totalIndices);

	size_t vertexOffset = 0;
	size_t indexOffset = 0;

	for (u32 i = 0; i < obj->face_count; i++)
	{
		for (u32 j = 0; j < obj->face_vertices[i]; j++)
		{
			fastObjIndex gi = obj->indices[indexOffset + j];
						
			Vertex v;
			v.pos =
			{
				obj->positions[gi.p * 3 + 0],
				obj->positions[gi.p * 3 + 1],
				obj->positions[gi.p * 3 + 2]
			};
			v.color =
			{
				obj->positions[gi.p * 3 + 0],
				obj->positions[gi.p * 3 + 1],
				obj->positions[gi.p * 3 + 2]
			};
			v.texCoord =
			{
				obj->texcoords[gi.t * 2 + 0],
				1.0f - obj->texcoords[gi.t * 2 + 1] // originally: obj->texcoords[gi.t * 2 + 1]
			};

			if (j >= 3)
			{
				vertices[vertexOffset + 0] = vertices[vertexOffset - 3];
				vertices[vertexOffset + 1] = vertices[vertexOffset - 1];
				vertexOffset += 2;
			}

			vertices[vertexOffset] = v;
			vertexOffset++;
		}

		indexOffset += obj->face_vertices[i];
	}

	fast_obj_destroy(obj);

	Mesh result;

	vector<u32> remap(totalIndices);

	size_t totalVertices = meshopt_generateVertexRemap(&remap[0], nullptr, totalIndices, &vertices[0], totalIndices, sizeof(Vertex));
	
	result.indices.resize(totalIndices);
	meshopt_remapIndexBuffer(&result.indices[0], nullptr, totalIndices, &remap[0]);

	result.vertices.resize(totalVertices);
	meshopt_remapVertexBuffer(&result.vertices[0], &vertices[0], totalIndices, sizeof(Vertex), &remap[0]);

	return result;
}