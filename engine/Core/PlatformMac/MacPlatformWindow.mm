//=============================================================================

#include "Core.h"

#define GLFW_EXPOSE_NATIVE_COCOA 1
#define GLFW_EXPOSE_NATIVE_NSGL 1
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

using namespace trillek::core::platform;


//-----------------------------------------------------------------------------
#pragma mark -  Creating Windows
//-----------------------------------------------------------------------------


MacWindow::MacWindow(const glm::vec2 size, const std::string &title)
: Window(size, title)
{
    // Force retina displays to create a 1x framebuffer so we don't choke our
    // fillrate.
    NSWindow *nativeWindow = glfwGetCocoaWindow((GLFWwindow*)this->glfwWindow);
    [[nativeWindow contentView] setWantsBestResolutionOpenGLSurface:NO];
}
