#include "Tilemap.h"

#include "Camera.h"

void Tilemap::Update(float deltaTime)
{

}

bool Tilemap::Initialize(DXRI* dxri, int size, float gridLen, glm::vec3 offset, bool slopedEdge)
{
	Object::Initialize(dxri);

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
			auto tmp = offset;
			if (slopedEdge)
			{
				if(x == 0)
				{
					v1.position.y += -5.0f;
					v3.position.y += -5.0f;
				}
				if(y == 0)
				{
					v1.position.y += -5.0f;
					v2.position.y += -5.0f;
				}
				if(x == size - 1)
				{
					v2.position.y += -5.0f;
					v4.position.y += -5.0f;
				}
				if(y == size - 1)
				{
					v3.position.y += -5.0f;
					v4.position.y += -5.0f;
				}

			}

			v1.position += offset;
			v2.position += offset;
			v3.position += offset;
			v4.position += offset;

			offset = tmp;

			vertices.push_back(v1);
			vertices.push_back(v3);
			vertices.push_back(v4);

			vertices.push_back(v1);
			vertices.push_back(v4);
			vertices.push_back(v2);

		}
	}



	mesh.loadFromVertices(dxri, vertices);
	return true;
}
