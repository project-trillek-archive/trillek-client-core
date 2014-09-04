//=============================================================================

#include "Core.h"

#include <GLFW/glfw3.h>

using namespace trillek::core::platform;


//-----------------------------------------------------------------------------
#pragma mark -  Creating Windows
//-----------------------------------------------------------------------------

Window::Window(const glm::vec2 size, const std::string &title)
{
    // In the future, this should be moved into the Render module.
    static std::once_flag OnceFlag;
    std::call_once(OnceFlag, []() {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    });
    
    this->glfwWindow = glfwCreateWindow(size.x, size.y, title.c_str(), NULL, NULL);
    glfwSetWindowUserPointer((GLFWwindow*)this->glfwWindow, this);
}


Window::~Window()
{
    glfwDestroyWindow((GLFWwindow*)this->glfwWindow);
}


//-----------------------------------------------------------------------------
#pragma mark -  Configuring Windows
//-----------------------------------------------------------------------------

void Window::setTitle(const std::string &title)
{
    glfwSetWindowTitle((GLFWwindow*)this->glfwWindow, title.c_str());
}


glm::vec2 Window::getPosition()
{
    int x, y;
    glfwGetWindowPos((GLFWwindow*)this->glfwWindow, &x, &y);
    return glm::vec2(x, y);
}
void Window::setPosition(const glm::vec2 newPosition)
{
    glfwSetWindowPos((GLFWwindow*)this->glfwWindow, newPosition.x, newPosition.y);
}


glm::vec2 Window::getSize()
{
    int width, height;
    glfwGetWindowSize((GLFWwindow*)this->glfwWindow, &width, &height);
    return glm::vec2(width, height);
}
void Window::setSize(const glm::vec2 newSize)
{
    glfwSetWindowSize((GLFWwindow*)this->glfwWindow, newSize.x, newSize.y);
}


void Window::hide()
{
    glfwHideWindow((GLFWwindow*)this->glfwWindow);
}
void Window::show()
{
    glfwShowWindow((GLFWwindow*)this->glfwWindow);
}