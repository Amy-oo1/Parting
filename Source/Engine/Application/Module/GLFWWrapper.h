#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_MODULE(GLFWWrapper)

#else
#pragma once

//#include "Core/ModuleBuild.h"

//Global


#ifndef PARTING_VULKAN_INCLUDE
#define PARTING_VULKAN_INCLUDE
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.h"
#include <vulkan/vulkan.hpp> 
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif // !VulkanInclude

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32

#include "ThirdParty/glfw-3.4/include/GLFW/glfw3.h"
#include "ThirdParty/glfw-3.4/include/GLFW/glfw3native.h"

//#include "ThirdParty/glfw-3.4/src/win32_platform.h"
//#include "ThirdParty/glfw-3.4/src/win32_joystick.h"
//#include "ThirdParty/glfw-3.4/src/win32_thread.h"

#endif // PARTING_MODULE_BUILD