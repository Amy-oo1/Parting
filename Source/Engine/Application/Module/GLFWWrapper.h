#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_MODULE(GLFWWrapper)

#else
#pragma once

//#include "Core/ModuleBuild.h"

//Global

#define GLFW_EXPOSE_NATIVE_WIN32
#include "ThirdParty/glfw-3.4/include/GLFW/glfw3.h"
#include "ThirdParty/glfw-3.4/include/GLFW/glfw3native.h"
//#include "ThirdParty/glfw-3.4/src/win32_platform.h"
//#include "ThirdParty/glfw-3.4/src/win32_joystick.h"
//#include "ThirdParty/glfw-3.4/src/win32_thread.h"

#endif // PARTING_MODULE_BUILD