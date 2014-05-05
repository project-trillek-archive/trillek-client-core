//=============================================================================

#include "Core.h"

#include <GLFW/glfw3.h>

using namespace trillek::core::platform;


//-----------------------------------------------------------------------------

Application::Application()
{
    // In the future, this should be moved into the Render module.
    if (glfwInit() != GL_TRUE) {
        throw new std::runtime_error("glfwInit() returned GL_FALSE");
    }
}


//-----------------------------------------------------------------------------
#pragma mark -  Message Loop
//-----------------------------------------------------------------------------

void Application::MessageLoop()
{
    glfwPollEvents();
}