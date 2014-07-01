#ifndef SIX_DOF_CAMERA_HPP_INCLUDED
#define SIX_DOF_CAMERA_HPP_INCLUDED

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "graphics/camera.hpp"

namespace trillek {
namespace graphics {

// This is a 6 degree-of-freedom camera allowing movement on any axis relative to the current orientation.
class SixDOFCamera :
    public CameraBase {
public:
    /**
     * \brief Computes the view matrix for a 6 DOF camera using the provided transform.
     *
     * \return void
     */
    glm::mat4 GetViewMatrix() {
        if (!this->camera_transform) {
            return glm::mat4(1.0f);
        }
        auto camera_translation = this->camera_transform->GetTranslation();
        auto camera_orientation = this->camera_transform->GetOrientation();
        return glm::lookAt(camera_translation,
            camera_translation + (camera_orientation * FORWARD_VECTOR),
            camera_orientation * UP_VECTOR);
    }

    /**
     * \brief Handles keyboard events.
     *
     * This method calls the physics system SetForce method after each event.
     * \return void
     */
    void Notify(const KeyboardEvent* key_event) {
        if (!this->camera_transform) {
            return;
        }

        static glm::vec3 direction_vector;
        static const physics::Force zero_gravity = { 0.0, 0.0, 0.0 };
        static bool gravity_disabled;

        switch (key_event->action) {
        case KeyboardEvent::KEY_DOWN:
            switch (key_event->key) {
            case GLFW_KEY_W:
                direction_vector += entity_speed * (FORWARD_VECTOR);
            break;
            case GLFW_KEY_S:
                direction_vector += -entity_speed * (FORWARD_VECTOR);
            break;
            case GLFW_KEY_A:
                direction_vector += -entity_speed * (RIGHT_VECTOR);
            break;
            case GLFW_KEY_D:
                direction_vector += entity_speed * (RIGHT_VECTOR);
            break;
            case GLFW_KEY_UP:
                this->camera_transform->OrientedRotate(glm::radians(5.0f) * RIGHT_VECTOR);
            break;
            case GLFW_KEY_DOWN:
                this->camera_transform->OrientedRotate(glm::radians(-5.0f) * RIGHT_VECTOR);
            break;
            case GLFW_KEY_LEFT:
                this->camera_transform->OrientedRotate(glm::radians(5.0f) * UP_VECTOR);
            break;
            case GLFW_KEY_RIGHT:
                this->camera_transform->OrientedRotate(glm::radians(-5.0f) * UP_VECTOR);
            break;
            default:
            break;
            }
        break;
        case KeyboardEvent::KEY_UP:
            switch (key_event->key) {
            case GLFW_KEY_W:
                direction_vector -= entity_speed * (FORWARD_VECTOR);
                break;
            case GLFW_KEY_S:
                direction_vector -= -entity_speed * (FORWARD_VECTOR);
                break;
            case GLFW_KEY_A:
                direction_vector -= -entity_speed * (RIGHT_VECTOR);
                break;
            case GLFW_KEY_D:
                direction_vector -= entity_speed * (RIGHT_VECTOR);
                break;
            case GLFW_KEY_UP:
                this->camera_transform->OrientedRotate(glm::radians(5.0f) * RIGHT_VECTOR);
            break;
            case GLFW_KEY_DOWN:
                this->camera_transform->OrientedRotate(glm::radians(-5.0f) * RIGHT_VECTOR);
            break;
            case GLFW_KEY_LEFT:
                this->camera_transform->OrientedRotate(glm::radians(5.0f) * UP_VECTOR);
            break;
            case GLFW_KEY_RIGHT:
                this->camera_transform->OrientedRotate(glm::radians(-5.0f) * UP_VECTOR);
            break;
            case GLFW_KEY_GRAVE_ACCENT:
                if (gravity_disabled) {
                    gravity_disabled = false;
                    TrillekGame::GetPhysicsSystem().SetGravity(this->entity_id, nullptr);
                }
                else {
                    gravity_disabled = true;
                    TrillekGame::GetPhysicsSystem().SetGravity(this->entity_id, &zero_gravity);
                }
            break;
            default:
            break;
            }
        break;
        default:
        break;
        }

        glm::vec3 move_vector = this->camera_transform->GetOrientation() * direction_vector;
        physics::Force force = { move_vector.x, move_vector.y, move_vector.z };
        TrillekGame::GetPhysicsSystem().SetForce(this->entity_id, force);
    }
private:
};


} // End of graphics
} // End of trillek
#endif
