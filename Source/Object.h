#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "ConstantBuffer.h"
#include "Mesh.h"
#include "Pipeline.h"

struct ObjectCB 
{
	glm::mat4 Model;
};

struct Object
{
	Mesh mesh;

	glm::vec3 location = glm::vec3(0,0,0);
	glm::vec3 scale = glm::vec3(1.0f);

	ObjectCB objCb;
	ConstantBuffer cb;
	UINT8* cbMapped = nullptr;

	virtual void Initialize();

	virtual void Update(float deltaTime) = 0;

	void Draw(ID3D12GraphicsCommandList* cmd, Pipeline& pipeline);
};
