#pragma once
#include "Object.h"

struct Unit : public Object
{
	virtual void Update(float deltaTime) override;

	void Initialize() override;
};
