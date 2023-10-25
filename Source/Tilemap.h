#pragma once

#include "Mesh.h"
#include "pch.h"

struct Tilemap 
{
	int size = 3;
	float gridLen = 5.0f;
	Mesh plane;

	bool Initialize(ID3D12Device* device);
};
