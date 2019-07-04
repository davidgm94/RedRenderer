#pragma once

#include "glfw.h"

void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	bool* pFramebufferResized = (bool*)glfwGetWindowUserPointer(window);
	*pFramebufferResized = true;
}
