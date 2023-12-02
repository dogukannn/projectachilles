#pragma once
#include "Object.h"

struct Unit : public Object
{
	glm::vec3 Target;

	virtual void Update(float deltaTime) override;
	virtual bool IsSelectable() override { return true; };
	void Initialize(DXRI* dxri) override;

	virtual void SetTarget(glm::vec3 target) override { Target = target; }
};
