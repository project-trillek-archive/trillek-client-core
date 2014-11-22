#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED

#include "trillek-game.hpp"
#include "components/shared-component.hpp"
#include "components/system-component.hpp"
#include "systems/transform-system.hpp"
#include "type-id.hpp"
#include "transform.hpp"
#include <memory>

namespace trillek {

namespace graphics {

using namespace component;

class CameraBase :
    public ComponentBase {
public:
    /**
     * \brief Activates this camera with the given entity ID.
     *
     * If you want to activate the camera with the same ID just pass in
     * CameraBase::GetEntityID();.
     * \param const unsigned int entity_id The ID of the entity whose transform we need to use.
     */
    void Activate(const unsigned int entity_id) {
        this->active = true;
        this->entity_id = entity_id;
        if (! Has<Component::Velocity>(entity_id)) {
            // Add velocity component if needed
            Insert<Component::Velocity>(entity_id, Velocity_type());
            // set null values to prevent move
            this->entity_speed = glm::vec3(0.0f);
            this->entity_rotation_speed = glm::vec3(0.0f);
        }
        else {
            auto& velocity = Get<Component::VelocityMax>(entity_id);
            this->entity_speed = velocity.linear;
            this->entity_rotation_speed = velocity.angular;
        }
        this->camera_transform = GetConstSharedPtr<Component::GraphicTransform>(entity_id);
    }

    /**
     * \brief Deactivates this camera.
     */
    void Deactivate() {
        this->active = false;
        this->camera_transform = nullptr;
        // if the entity can't move, remove the velocity component
        if (this->entity_speed == glm::vec3(0.0f)) {
            Remove<Component::Velocity>(entity_id);
        }
    }

    virtual bool Initialize(const std::vector<Property> &properties) {
        return true;
    }

    /**
     * \brief Returns if this camera is active.
     */
    bool IsActive() {
        return this->active;
    }

    /**
     * \brief Get the current camera's entity ID.
     */
    unsigned int GetEntityID() {
        return this->entity_id;
    }

    void UpdateTransform(std::shared_ptr<const Transform> transform) {
        camera_transform = std::move(transform);
    }

    /**
     * \brief Compute and return the camera's view matrix.
     *
     * \return glm::mat4 The computed view matrix, or identity matrix if the camera's entity transform is null.
     */
    virtual glm::mat4 GetViewMatrix() { return glm::mat4(1.0f); };
protected:
    std::shared_ptr<const Transform> camera_transform;
    unsigned int entity_id;
    glm::vec3 entity_speed;
    glm::vec3 entity_rotation_speed;
    bool active;
};

} // End of graphics

namespace reflection {
TRILLEK_MAKE_IDTYPE_NS(graphics, CameraBase, 2002)
} // namespace reflection

} // End of trillek
#endif
