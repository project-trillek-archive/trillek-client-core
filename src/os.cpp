#include "os.hpp"

#include <iostream>
#include "trillek-game.hpp"
#include "systems/dispatcher.hpp"

#ifdef __APPLE__
// Needed so we can disable retina support for our window.
#define GLFW_EXPOSE_NATIVE_COCOA 1
#define GLFW_EXPOSE_NATIVE_NSGL 1
#include <GLFW/glfw3native.h>
// We can't just include objc/runtime.h and objc/message.h because glfw is too forward thinking for its own good.
typedef void* SEL;
extern "C" id objc_msgSend(id self, SEL op, ...);
extern "C" SEL sel_getUid(const char *str);
#endif

namespace trillek {

// Error helper function used by GLFW for error messaging.
// Currently outputs to std::cout.
static void ErrorCallback(int error, const char* description) {
    std::cout << description << std::endl;
}

bool OS::InitializeWindow(const int width, const int height, const std::string title,
    const unsigned int glMajor /*= 3*/, const unsigned int glMinor /*= 2*/) {
    glfwSetErrorCallback(ErrorCallback);

    // Initialize the library.
    if (glfwInit() != GL_TRUE) {
        return false;
    }

    // don't hint much (request default context version)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

    // Create a windowed mode window and its OpenGL context.
    this->window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

    if (!this->window) {
        // creating a window with default hints failed
        // try again with specific hints
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        this->window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

        if (!this->window) {
            // still not right, give up
            glfwTerminate();
            return false;
        }
    }

    // attach the context
    glfwMakeContextCurrent(this->window);

    // check the context version
    std::string glcx_version((char*)glGetString(GL_VERSION));
    std::string glcx_major = glcx_version.substr(0, glcx_version.find('.', 0));
    if(glcx_major == "1") {
        // we got a version 1 context, that will not work
        // so try again.
        glfwMakeContextCurrent(nullptr);
        glfwDestroyWindow(this->window);
        // be more specific this time
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        this->window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

        if (!this->window) {
            glfwTerminate();
            return false;
        }
        // attach the context
        glfwMakeContextCurrent(this->window);

        // check the context version again
        glcx_version = (char*)glGetString(GL_VERSION);
        glcx_major = glcx_version.substr(0, glcx_version.find('.', 0));
        if(glcx_major == "1") {
            // still 1, higher versions probably not supported
            glfwTerminate();
            std::cerr << "Initializing OpenGL failed, unsupported version: " << glcx_version << '\n';
            std::cerr << "Press \"Enter\" to exit\n";
            std::cin.get();
            return false;
        }
    }

    this->client_width = width;
    this->client_height = height;

#ifdef __APPLE__
    // Force retina displays to create a 1x framebuffer so we don't choke our fillrate.
    id cocoaWindow = glfwGetCocoaWindow(this->window);
    id cocoaGLView = ((id (*)(id, SEL)) objc_msgSend)(cocoaWindow, sel_getUid("contentView"));
    ((void (*)(id, SEL, bool)) objc_msgSend)(cocoaGLView, sel_getUid("setWantsBestResolutionOpenGLSurface:"), false);
#endif

#ifndef __APPLE__
    // setting glewExperimental fixes a glfw context problem
    // (tested on Ubuntu 13.04)
    glewExperimental = GL_TRUE;

    // Init GLEW.
    GLuint error = glewInit();
    if (error != GLEW_OK) {
        return false;
    }
#endif

    // Associate a pointer for this instance with this window.
    glfwSetWindowUserPointer(this->window, this);

    // Set up some callbacks.
    glfwSetWindowSizeCallback(this->window, &OS::windowResized);
    glfwSetKeyCallback(this->window, &OS::keyboardEvent);
    glfwSetCursorPosCallback(this->window, &OS::mouseMoveEvent);
    glfwSetCharCallback(this->window, &OS::characterEvent);
    glfwSetMouseButtonCallback(this->window, &OS::mouseButtonEvent);
    glfwSetWindowFocusCallback(this->window, &OS::windowFocusChange);

    glfwGetCursorPos(this->window, &this->old_mouse_x, &this->old_mouse_y);

    return true;
}

void OS::MakeCurrent() {
    glfwMakeContextCurrent(this->window);
}

void OS::DetachContext() {
    glfwMakeContextCurrent(NULL);
}

void OS::Terminate() {
    glfwTerminate();
}

void OS::SetWindowShouldClose() {
    glfwSetWindowShouldClose(this->window, GL_TRUE);
}

bool OS::Closing() {
    return glfwWindowShouldClose(this->window) > 0;
}

void OS::SwapBuffers() {
    glfwSwapBuffers(this->window);
}

void OS::OSMessageLoop() {
    glfwWaitEvents();
}

int OS::GetWindowWidth() {
    return this->client_width;
}

int OS::GetWindowHeight() {
    return this->client_height;
}

std::chrono::nanoseconds OS::GetTime() {
    return std::chrono::nanoseconds(static_cast<int64_t>(glfwGetTime() * 1.0E9));
}

void OS::windowResized(GLFWwindow* window, int width, int height) {
    // Get the user pointer and cast it.
    OS* os = static_cast<OS*>(glfwGetWindowUserPointer(window));

    if (os) {
        os->UpdateWindowSize(width, height);
    }
}

void OS::keyboardEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Get the user pointer and cast it.
    OS* os = static_cast<OS*>(glfwGetWindowUserPointer(window));

    if (os) {
        os->DispatchKeyboardEvent(key, scancode, action, mods);
    }
}

void OS::characterEvent(GLFWwindow* window, unsigned int uchar) {
    // Get the user pointer and cast it.
    OS* os = static_cast<OS*>(glfwGetWindowUserPointer(window));

    if (os) {
        os->DispatchCharacterEvent(uchar);
    }
}

void OS::mouseMoveEvent(GLFWwindow* window, double x, double y) {
    // Get the user pointer and cast it.
    OS* os = static_cast<OS*>(glfwGetWindowUserPointer(window));

    if (os) {
        os->DispatchMouseMoveEvent(x, y);
    }
}

void OS::mouseButtonEvent(GLFWwindow* window, int button, int action, int mods) {
    // Get the user pointer and cast it.
    OS* os = static_cast<OS*>(glfwGetWindowUserPointer(window));

    if (os) {
        os->DispatchMouseButtonEvent(button, action, mods);
    }
}

void OS::windowFocusChange(GLFWwindow* window, int focused) {
    if (focused == GL_FALSE) {
        // Get the user pointer and cast it.
        OS* os = static_cast<OS*>(glfwGetWindowUserPointer(window));

        if (os) {
            // TODO: Implement a DispatchWindowFocusEvent() method in OS
            // TODO: Dispatch a focus changed event.
        }
    }
}

void OS::UpdateWindowSize(const int width, const int height) {
    this->client_width = width;
    this->client_height = height;
}

void OS::DispatchKeyboardEvent(const int key, const int scancode, const int action, const int mods) {
    KeyboardEvent key_event;
    if (action == GLFW_PRESS) {
        key_event = { key, scancode, KeyboardEvent::KEY_DOWN, mods };
    }
    else if (action == GLFW_REPEAT) {
        key_event = { key, scancode, KeyboardEvent::KEY_REPEAT, mods };
    }
    else if (action == GLFW_RELEASE) {
        key_event = { key, scancode, KeyboardEvent::KEY_UP, mods };
    }

    event::Dispatcher<KeyboardEvent>::GetInstance()->NotifySubscribers(&key_event);
}

void OS::DispatchCharacterEvent(const unsigned int uchar) {
    KeyboardEvent key_event { (const int)uchar, 0, KeyboardEvent::KEY_CHAR, 0 };
    event::Dispatcher<KeyboardEvent>::GetInstance()->NotifySubscribers(&key_event);
}

void OS::DispatchMouseMoveEvent(const double x, const double y) {
    MouseMoveEvent mmov_event = {
        static_cast<double>(x) / this->client_width,
        static_cast<double>(y) / this->client_height,
        static_cast<int>(this->old_mouse_x),
        static_cast<int>(this->old_mouse_y),
        static_cast<int>(x),
        static_cast<int>(y)
    };
    event::Dispatcher<MouseMoveEvent>::GetInstance()->NotifySubscribers(&mmov_event);

    // If we are in mouse lock we will snap the mouse to the middle of the screen.
    if (this->mouse_lock) {
        this->old_mouse_x = this->client_width / 2;
        this->old_mouse_y = this->client_height / 2;
        glfwSetCursorPos(this->window, this->old_mouse_x, this->old_mouse_y);
    }
    else {
        this->old_mouse_x = x;
        this->old_mouse_y = y;
    }
}

void OS::DispatchMouseButtonEvent(const int button, const int action, const int mods) {
    MouseBtnEvent mbtn_event;
    if (action == GLFW_PRESS) {
        mbtn_event.action = MouseBtnEvent::DOWN;
    }
    else {
        mbtn_event.action = MouseBtnEvent::UP;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mbtn_event.button = MouseBtnEvent::LEFT;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        mbtn_event.button = MouseBtnEvent::RIGHT;
    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        mbtn_event.button = MouseBtnEvent::MIDDLE;
    }
    event::Dispatcher<MouseBtnEvent>::GetInstance()->NotifySubscribers(&mbtn_event);

}

void OS::ToggleMouseLock() {
    this->mouse_lock = !this->mouse_lock;
    if (this->mouse_lock) {
        glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else {
        glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void OS::SetMousePosition(double x, double y) {
    glfwSetCursorPos(this->window, x, y);
}

} // End of trillek
