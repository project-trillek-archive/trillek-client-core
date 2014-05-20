#ifndef GRAPHICS_HPP_INCLUDED
#define GRAPHICS_HPP_INCLUDED

#ifndef __APPLE__
#include <GL/glew.h>
#endif
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <list>
#include <memory>
#include <vector>

namespace trillek {
namespace graphics {

class Material;
class Renderable;

struct MaterialGroup {
    std::shared_ptr<Material> material;
    // Each vector contains a list of renderables.
    // The vector's index matches the texture index in the material.
    std::vector<std::list<std::shared_ptr<Renderable>>> renderables;
};

class System {
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

    /**
    * \brief Adds a Renderable component to the system..
    *
    * \param const unsigned int entityID The entity ID the compoennt belongs to.
    * \param std::shared_ptr<Renderable> ren The Renderable to add.
    * \return void
    */
    void AddRenderable(const unsigned int entity_id, std::shared_ptr<Renderable> ren);

    /**
    * \brief Removes a Renderable component from the system..
    *
    * \param const unsigned int entityID The entity ID of the compoennt to remove.
    * \return void
    */
    void RemoveRenderable(const unsigned int entity_id);
private:
    int gl_version[2];
    glm::mat4 projection_matrix;
    glm::mat4 view_matrix;

    unsigned int window_width; // Store the width of our window
    unsigned int window_height; // Store the height of our window

    // A list of the renderables in the system. Stored as a pair (entity ID, Renderable).
    std::list<std::pair<unsigned int, std::shared_ptr<Renderable>>> renderables;
    std::list<MaterialGroup> material_groups;
};

} // End of graphics
} // End of trillek

#endif
