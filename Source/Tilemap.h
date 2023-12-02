#pragma once

#include "Object.h"
#include "pch.h"
#include <glm/vec3.hpp>

struct Tilemap : public Object
{
	int size = 49;
	float gridLen = 5.0f;

	virtual void Update(float deltaTime) override;

	bool Initialize(::DXRI* dxri, int size, float gridLen, glm::vec3 offset = glm::vec3(0.,0.,0.), bool slopedEdge = false);
};
