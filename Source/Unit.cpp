#include "Unit.h"

void Unit::Update(float deltaTime)
{
	glm::vec3 speed(1, 0, 1);

	location += speed * deltaTime * 0.002f;

}

void Unit::Initialize()
{
	Object::Initialize();
	mesh.loadFromObj(GDevice, "../Assets/cube.obj");

	location = glm::vec3(0, 1, 0);
}
