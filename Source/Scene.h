#pragma
#include "Object.h"

struct Scene
{
	std::vector<Object*> Objects;

	void Draw(ID3D12GraphicsCommandList* cmd, Pipeline& pipeline);
	void Update(float deltaTime);
};

