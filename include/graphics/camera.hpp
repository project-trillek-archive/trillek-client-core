#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED

#include "systems/transform-system.hpp"
#include "component.hpp"
#include "type-id.hpp"
#include "transform.hpp"
#include <memory>

namespace trillek {
namespace graphics {

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
        this->entity_speed = 10.0f;
        this->entity_rotation_speed = 5.0f;
        this->camera_transform = TransformMap::GetTransform(entity_id);
    }

    /**
     * \brief Deactivates this camera.
     */
    void Deactivate() {
        this->active = false;
        this->camera_transform = nullptr;
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

    /**
     * \brief Compute and return the camera's view matrix.
     *
     * \return glm::mat4 The computed view matrix, or identity matrix if the camera's entity transform is null.
     */
    virtual glm::mat4 GetViewMatrix() = 0;
protected:
    std::shared_ptr<Transform> camera_transform;
    unsigned int entity_id;
    float entity_speed;
    float entity_rotation_speed;
    bool active;
};

} // End of graphics

namespace reflection {
TRILLEK_MAKE_IDTYPE_NS(graphics, CameraBase, 2002)
} // namespace reflection

} // End of trillek
#endif
