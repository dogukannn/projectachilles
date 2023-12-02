#include "Unit.h"

void Unit::Update(float deltaTime)
{
	//glm::vec3 speed(1, 0, 1);
	if(glm::distance(Target, location) < 0.01)
		return;

	auto dir = normalize((Target - location));
	location += dir * deltaTime * 0.002f;

}

void Unit::Initialize(DXRI* dxri)
{
	Target = glm::vec3(5, 0, 5);

	Object::Initialize(dxri);
	mesh.loadFromObj(dxri, "../Assets/cube.obj");

	location = glm::vec3(0, 1, 0);
}
