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

struct ObjectTintCB 
{
	glm::vec3 tint;
};

struct Object
{
	Mesh mesh;
	std::string name;

	glm::vec3 location = glm::vec3(0,0,0);
	glm::vec3 scale = glm::vec3(1.0f);
	bool selected = false;

	ObjectCB objCb;
	ConstantBuffer cb;
	UINT8* cbMapped = nullptr;
	
	ObjectTintCB tintCb;
	ConstantBuffer tintcb;
	UINT8* tintcbMapped = nullptr;

	virtual void Initialize(DXRI* dxri);

	glm::mat4 GetModelMatrix();

	virtual void Update(float deltaTime) = 0;

	virtual bool IsSelectable() { return false; }

	virtual void SetTarget(glm::vec3 target) {}

	void Draw(ID3D12GraphicsCommandList* cmd, Pipeline& pipeline);
};
