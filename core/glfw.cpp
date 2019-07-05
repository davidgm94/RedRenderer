#pragma once
#include "common.h"
#include "glfw.h"

void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	bool32* pFramebufferResized = (bool32*)glfwGetWindowUserPointer(window);
	*pFramebufferResized = true;
}
