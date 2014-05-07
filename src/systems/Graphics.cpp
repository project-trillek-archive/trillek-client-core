#include "systems/Graphics.hpp"
namespace trillek {
namespace graphics {
const int* GL::Start(const unsigned int width, const unsigned int height) {
    // Use the GL3 way to get the version number
    glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);

    // Sanity check to make sure we are at least in a good major version number.
    assert((OpenGLVersion[0] > 1) && (OpenGLVersion[0] < 5));

    SetViewportSize(width, height);

    // Set a default view that is back and up from the center.
    this->view = glm::lookAt(glm::vec3(0.0f, -10.0f, 4.0f),
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 1.0f));

    // App specific global gl settings
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
    return OpenGLVersion;
}

void GL::Update(const double delta) {
    // Clear the backbuffer and primary depth/stencil buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, this->windowWidth, this->windowHeight); // Set the viewport size to fill the window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers

}

void GL::SetViewportSize(const unsigned int width, const unsigned int height) {
    this->windowHeight = height;
    this->windowWidth = width;

    // Determine the aspect ratio and sanity check it to a safe ratio
    float aspectRatio = static_cast<float>(this->windowWidth) / static_cast<float>(this->windowHeight);
    if (aspectRatio < 1.0f) {
        aspectRatio = 4.0f / 3.0f;
    }

    // update projection matrix based on new aspect ratio
    this->proj = glm::perspective(
        45.0f,
        aspectRatio,
        0.1f,
        10000.0f
        );
}

}
}
