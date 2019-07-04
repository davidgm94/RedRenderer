#pragma once
#ifndef GLFW
#define GLFW

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

void framebufferResizeCallback(GLFWwindow* window, int width, int height);
#endif