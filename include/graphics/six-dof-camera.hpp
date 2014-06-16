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

    void Notify(const unsigned int entity_id, const KeyboardEvent* key_event) {
        if (!this->camera_transform) {
            return;
        }
        switch (key_event->action) {
        case KeyboardEvent::KEY_REPEAT:
        case KeyboardEvent::KEY_DOWN:
            switch (key_event->key) {
            case GLFW_KEY_W:
                this->camera_transform->OrientedTranslate(1.0f * FORWARD_VECTOR);
            break;
            case GLFW_KEY_S:
                this->camera_transform->OrientedTranslate(-1.0f * FORWARD_VECTOR);
            break;
            case GLFW_KEY_A:
                this->camera_transform->OrientedTranslate(-1.0f * RIGHT_VECTOR);
            break;
            case GLFW_KEY_D:
                this->camera_transform->OrientedTranslate(1.0f * RIGHT_VECTOR);
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
        default:
        break;
        }
    }
private:
};


} // End of graphics
} // End of trillek
#endif
