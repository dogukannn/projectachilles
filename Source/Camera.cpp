#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

Camera::Camera()
{
	fov = glm::radians(46.f);
	aspect = 1.33;
	nearz = 1.0f;
	farz = 1000.0f;
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

