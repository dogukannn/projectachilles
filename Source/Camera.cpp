#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(Eye, Eye + EyeDir, Up);
}

glm::mat4 Camera::GetPerspectiveMatrix()
{
	auto projectionMatrix = glm::perspective(glm::radians(46.f), 1.33f, 1.0f, 1000.f); // defined GLM_DEPTH_ZERO_TO_ONE for dx12s 0 to 1 depth
	return projectionMatrix;
}

glm::mat4 Camera::GetVPMatrix()
{
	auto projectionMatrix = glm::perspective(glm::radians(46.f), 1.33f, 1.0f, 1000.f); // defined GLM_DEPTH_ZERO_TO_ONE for dx12s 0 to 1 depth
	auto viewMatrix =  glm::lookAt(Eye, Eye + EyeDir, Up);
	return projectionMatrix * viewMatrix;
}

