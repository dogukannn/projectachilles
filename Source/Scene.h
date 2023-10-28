#pragma once
#include "Camera.h"
#include "Object.h"

struct Scene
{
	std::vector<Object*> Objects;

	void Draw(ID3D12GraphicsCommandList* cmd, Pipeline& pipeline);
	void Update(float deltaTime);

	void SetTargetOfSelectedUnits(glm::vec3 target);
	void SelectUnits(glm::vec2 start, glm::vec2 end, Camera& cam);
};

