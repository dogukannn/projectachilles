#include "Tilemap.h"

bool Tilemap::Initialize(ID3D12Device* device)
{
	std::vector<Vertex> vertices;

	for (int x = 0; x < size; x++)
	{
		for (int y = 0; y < size; y++)
		{
			float r = (rand() % 255) / 255.0f;
			float g = (rand() % 255) / 255.0f;
			float b = (rand() % 255) / 255.0f;
			
			Vertex v1 = { glm::vec3(x*gridLen, 0, y*gridLen), glm::vec3(0,0,1.0f), glm::vec3(r,g,b), glm::vec2(0,0) };
			Vertex v2 = { glm::vec3((x+1)*gridLen, 0, y*gridLen), glm::vec3(0,0,1.0f), glm::vec3(r,g,b), glm::vec2(0,0) };
			Vertex v3 = { glm::vec3(x*gridLen, 0, (y+1)*gridLen), glm::vec3(0,0,1.0f), glm::vec3(r,g,b), glm::vec2(0,0) };
			Vertex v4 = { glm::vec3((x+1)*gridLen, 0, (y+1)*gridLen), glm::vec3(0,0,1.0f), glm::vec3(r,g,b), glm::vec2(0,0) };

			vertices.push_back(v1);
			vertices.push_back(v3);
			vertices.push_back(v4);

			vertices.push_back(v1);
			vertices.push_back(v4);
			vertices.push_back(v2);

		}
	}

	plane.loadFromVertices(device, vertices);
	return true;
}
