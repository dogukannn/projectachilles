#include "Unit.h"

void Unit::Update(float deltaTime)
{
	//glm::vec3 speed(1, 0, 1);
	auto dir = normalize((Target - location));


	location += dir * deltaTime * 0.002f;

}

void Unit::Initialize()
{
	Target = glm::vec3(5, 0, 5);

	Object::Initialize();
	mesh.loadFromObj(GDevice, "../Assets/cube.obj");

	location = glm::vec3(0, 1, 0);
}
