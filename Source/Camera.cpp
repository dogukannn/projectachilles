#include "Camera.h"

#include <iostream>
#include <ostream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <SDL_scancode.h>
#include <SDL_mouse.h>


Camera::Camera()
{
	fov = glm::radians(46.f);
	aspect = 1.33;
	nearz = 1.0f;
	farz = 1000.0f;
}

void Camera::HandleInput(const Uint8* sdl_keyboard_state, int mouseX, int mouseY, bool windowFocused, float deltaTime)
{

	if (sdl_keyboard_state[SDL_SCANCODE_F])
	{
		freeCamState = true;
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	if (sdl_keyboard_state[SDL_SCANCODE_M])
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
		freeCamState = false;
	}


	if(!freeCamState)
	{
		if(sdl_keyboard_state[SDL_SCANCODE_D])
			Eye += glm::normalize(glm::cross(EyeDir, Up)) * speed * deltaTime;
		if(sdl_keyboard_state[SDL_SCANCODE_W])
			Eye += glm::normalize(EyeDir) * speed * deltaTime;
		if(sdl_keyboard_state[SDL_SCANCODE_S])
			Eye -= glm::normalize(EyeDir) * speed * deltaTime;
		if(sdl_keyboard_state[SDL_SCANCODE_A])
			Eye -= glm::normalize(glm::cross(EyeDir, Up)) * speed * deltaTime;

		if(windowFocused)
		{
			int xw, yw;
			xw = mouseX;
			yw = mouseY;
			//std::cout << xw <<  ", " << yw << std::endl;
			if(xw < 10)
			{
				auto right = normalize(glm::cross(EyeDir, Up)) * speed * deltaTime;
				Eye -= glm::vec3(right.x, 0, right.z);
			}
			else if(xw > 790)
			{
				auto right = normalize(glm::cross(EyeDir, Up)) * speed * deltaTime;
				Eye += glm::vec3(right.x, 0, right.z);
			}
			if(yw < 10)
			{
				auto forward = normalize(EyeDir) * speed * deltaTime;
				Eye += glm::vec3(forward.x, 0, forward.z);
			}
			else if(yw > 790)
			{
				auto forward = normalize(EyeDir) * speed * deltaTime;
				Eye -= glm::vec3(forward.x, 0, forward.z);
			}
		}
	}
	else
	{
		if (sdl_keyboard_state[SDL_SCANCODE_W])
			Eye += glm::normalize(EyeDir) * speed * deltaTime;
		if (sdl_keyboard_state[SDL_SCANCODE_S])
			Eye -= glm::normalize(EyeDir) * speed * deltaTime;

		if (sdl_keyboard_state[SDL_SCANCODE_D])
			Eye += glm::normalize(glm::cross(EyeDir, Up)) * speed * deltaTime;
		if (sdl_keyboard_state[SDL_SCANCODE_A])
			Eye -= glm::normalize(glm::cross(EyeDir, Up)) * speed * deltaTime;

		int relX, relY;
		SDL_GetRelativeMouseState(&relX, &relY);

		auto right = glm::normalize(glm::cross(EyeDir, Up));

		if ((relY < 0) && (glm::dot(EyeDir, Up) < 0.99f) || (relY > 0) && (glm::dot(EyeDir, Up) > -0.99f))
			EyeDir = glm::rotate(glm::mat4(1.0f), 0.001f * (float) -relY, right) * glm::vec4(EyeDir, 0.0f);

		EyeDir = glm::rotate(glm::mat4(1.0f), 0.001f * (float) -relX, Up) * glm::vec4(EyeDir, 0.0f);
	}
}

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(Eye, Eye + EyeDir, Up);
}

glm::mat4 Camera::GetPerspectiveMatrix()
{
	auto projectionMatrix = glm::perspective(fov, aspect, nearz, farz); // defined GLM_DEPTH_ZERO_TO_ONE for dx12s 0 to 1 depth
	return projectionMatrix;
}

glm::mat4 Camera::GetVPMatrix()
{
	auto projectionMatrix = glm::perspective(fov, aspect, nearz, farz); // defined GLM_DEPTH_ZERO_TO_ONE for dx12s 0 to 1 depth
	auto viewMatrix =  glm::lookAt(Eye, Eye + EyeDir, Up);
	return projectionMatrix * viewMatrix;
}

