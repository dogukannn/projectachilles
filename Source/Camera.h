#pragma once

#include <SDL_stdinc.h>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "pch.h"

struct Camera 
{
	glm::vec3 Eye;
	glm::vec3 EyeDir;
	glm::vec3 Up;

	float fov;
	float aspect;
	float nearz;
	float farz;

    float speed = 0.03f;

	bool freeCamState = false;

	Camera();

	void HandleInput(const Uint8* sdl_keyboard_state, int mouseX, int mouseY, bool windowFocused, float deltaTime);

	glm::mat4 GetViewMatrix();
	glm::mat4 GetPerspectiveMatrix();
	glm::mat4 GetVPMatrix();

};