#ifndef GRAPHICS_HPP_INCLUDED
#define GRAPHICS_HPP_INCLUDED

#include "opengl.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <list>
#include <memory>
#include <vector>
#include "trillek.hpp"
#include "trillek-scheduler.hpp"
#include "systems/system-base.hpp"
#include "util/json-parser.hpp"
#include "graphics/graphics-base.hpp"
#include "graphics/material.hpp"
#include <map>

#include "dispatcher.hpp"
namespace trillek {

class Transform;

namespace graphics {

class Renderable;
class CameraBase;

struct MaterialGroup {
    Material material;
    struct TextureGroup {
        std::vector<size_t> texture_indicies;
        struct RenderableGroup {
            std::shared_ptr<Renderable> renderable;
            std::list<unsigned int> instances;
            size_t buffer_group_index;
        };
        std::list<RenderableGroup> renderable_groups;
    };
    std::list<TextureGroup> texture_groups;
};

class RenderSystem : public SystemBase,
    public event::Subscriber<Transform>,
    public util::Parser {
public:

    RenderSystem();

    // Inherited from Parser
    virtual bool Serialize(rapidjson::Document& document);

    // Inherited from Parser
    virtual bool Parse(rapidjson::Value& node);

    /**
     * \brief Starts the OpenGL rendering system.
     *
     * Prepares the OpengGL rendering context enabling and disabling certain flags.
     * \param const unsigned int width Initial viewport width.
     * \param const unsigned int height Initial viewport height.
     * \return const int*[2] -1 in the 0 index on failure, else the major and minor version in index 0 and 1 respectively.
     */
    const int* Start(const unsigned int width, const unsigned int height);

    /** \brief Makes the context of the window current to the thread
     *
     */
    void ThreadInit() override;

    /**
     * \brief Causes an update in the system based on the change in time.
     *
     * Updates the state of the system based off how much time has elapsed since the last update.
     * \return void
     */
    // TODO: This is a niave update render method. Please replace me.
    void RunBatch() const override;

    /**
     * \brief Notification that a transform has changed.
     *
     * \param const unsigned int entity_id ID of the entity to update.
     * \param const Transform* transform The entity's transform.
     * \return void
     */
    void Notify(const unsigned int entity_id, const Transform* transform);

    /**
     * \brief Sets the viewport width and height.
     *
     * \param const unsigned int width New viewport width
     * \param const unsigned int height New viewport height
     * \return void
     */
    void SetViewportSize(const unsigned int width, const unsigned int height);

    /**
     * \brief Adds a renderable component to the system.
     *
     * A static_pointer_case is applied to the component shared_ptr to cast it to
     * a Renderable component. If the cast results in a nullptr the method returns
     * without adding the renderable component.
     * \param const unsigned int entityID The entity ID the compoennt belongs to.
     * \param std::shared_ptr<ComponentBase> component The component to add.
     * \return void
     */
    void AddComponent(const unsigned int entity_id, std::shared_ptr<ComponentBase> component);

    /**
     * \brief Removes a Renderable component from the system..
     *
     * \param const unsigned int entityID The entity ID of the compoennt to remove.
     * \return void
     */
    void RemoveRenderable(const unsigned int entity_id);

    /** \brief Handle incoming events to update data
     *
     * This function is called once every frame. It is the only
     * function that can write data. This function is in the critical
     * path, job done here must be simple.
     *
     * If event handling need some batch processing, a task list must be
     * prepared and stored temporarily to be retrieved by RunBatch().
     *
     */
    void HandleEvents(const frame_tp& timepoint) override {
        // TODO
    };

    /** \brief Save the data and terminate the system
     *
     * This function is called when the program is closing
     *
     */
    void Terminate() override;

    template<class T>
    std::shared_ptr<T> Get(const std::string & instancename) {
        unsigned int type_id = reflection::GetTypeID<T>();
        auto instance_ptr = this->graphics_instances[type_id].find(instancename);
        if(instance_ptr == this->graphics_instances[type_id].end()) {
            return std::shared_ptr<T>();
        }
        return std::static_pointer_cast<T>(instance_ptr->second);
    }
    template<typename T>
    void Add(const std::string & instancename, std::shared_ptr<T> instanceptr) {
        unsigned int type_id = reflection::GetTypeID<T>();
        graphics_instances[type_id][instancename] = instanceptr;
    }
private:
//	bool ParseShader(const std::string &shader_name, rapidjson::Value& node);

    int gl_version[2];
    glm::mat4 projection_matrix;
    glm::mat4 view_matrix;

    std::shared_ptr<CameraBase> camera;

    unsigned int window_width; // Store the width of our window
    unsigned int window_height; // Store the height of our window

    // A list of the renderables in the system. Stored as a pair (entity ID, Renderable).
    std::list<std::pair<unsigned int, std::shared_ptr<Renderable>>> renderables;
    std::map<unsigned int, std::map<std::string, std::shared_ptr<GraphicsBase>>> graphics_instances;
    std::map<unsigned int, glm::mat4> model_matrices;
    std::list<MaterialGroup> material_groups;
};

} // End of graphics
} // End of trillek

#endif
