#ifndef GRAPHICS_HPP_INCLUDED
#define GRAPHICS_HPP_INCLUDED

#ifndef __APPLE__
#include <GL/glew.h>
#endif
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace trillek {
namespace graphics {
class GL {
public:

    /**
    * \brief Starts the OpenGL rendering system.
    *
    * Prepares the OpengGL rendering context enabling and disabling certain flags.
    * \param const unsigned int width Initial viewport width.
    * \param const unsigned int height Initial viewport height.
    * \return const int*[2] -1 in the 0 index on failure, else the major and minor version in index 0 and 1 respectively.
    */
    const int* Start(const unsigned int width, const unsigned int height);

    /**
    * \brief Causes an update in the system based on the change in time.
    *
    * Updates the state of the system based off how much time has elapsed since the last update.
    * \param const double delta The time (in seconds) since the last update
    * \return void
    */
    // TODO: This is a niave update render method. Please replace me.
    void Update(const double delta);

    /**
    * \brief Sets the viewport width and height.
    *
    * \param const unsigned int width New viewport width
    * \param const unsigned int height New viewport height
    * \return void
    */
    void SetViewportSize(const unsigned int width, const unsigned int height);
private:
    int OpenGLVersion[2];
    glm::mat4 proj;
    glm::mat4 view;

    unsigned int windowWidth; // Store the width of our window
    unsigned int windowHeight; // Store the height of our window
};

}
}

#endif
