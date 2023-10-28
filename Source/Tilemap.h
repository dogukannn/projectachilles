#pragma once

#include "Object.h"
#include "pch.h"

struct Tilemap : public Object
{
	int size = 49;
	float gridLen = 5.0f;

	virtual void Update(float deltaTime) override;

	bool Initialize(int size, float gridLen);
};
