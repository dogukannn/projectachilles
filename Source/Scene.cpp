#include "Scene.h"

void Scene::Draw(ID3D12GraphicsCommandList* cmd, Pipeline& pipeline)
{
	for(auto& object : Objects)
	{
		object->Draw(cmd, pipeline);
	}
}

void Scene::Update(float deltaTime)
{
	for(auto& object : Objects)
	{
		object->Update(deltaTime);
	}
}
