#include "graphics/six-dof-camera.hpp"
#include "trillek-game.hpp"
#include "os-event.hpp"
#include "os.hpp"
#include "systems/physics.hpp"

namespace trillek {
namespace graphics {

glm::mat4 SixDOFCamera::GetViewMatrix() {
    auto camera_translation = this->camera_transform->GetTranslation();
    auto camera_orientation = this->camera_transform->GetOrientation();
    return glm::lookAt(camera_translation,
        camera_translation + (camera_orientation * FORWARD_VECTOR),
        camera_orientation * UP_VECTOR);
}

void SixDOFCamera::Notify(const KeyboardEvent* key_event) {
    static glm::vec3 direction_vector;
    static glm::vec3 rotation_vector;
    static const btVector3 zero_gravity = { 0.0, 0.0, 0.0 };
    static bool gravity_disabled;

    switch (key_event->action) {
    case KeyboardEvent::KEY_DOWN:
        switch (key_event->key) {
        case GLFW_KEY_W:
            direction_vector += entity_speed.z * (FORWARD_VECTOR);
        break;
        case GLFW_KEY_S:
            direction_vector -= entity_speed.z * (FORWARD_VECTOR);
        break;
        case GLFW_KEY_A:
            direction_vector -= entity_speed.x * (RIGHT_VECTOR);
        break;
        case GLFW_KEY_D:
            direction_vector += entity_speed.x * (RIGHT_VECTOR);
        break;
        case GLFW_KEY_UP:
            rotation_vector += entity_rotation_speed.x * (RIGHT_VECTOR);
        break;
        case GLFW_KEY_DOWN:
            rotation_vector -= entity_rotation_speed.x * (RIGHT_VECTOR);
        break;
        case GLFW_KEY_LEFT:
            rotation_vector += entity_rotation_speed.y * (UP_VECTOR);
        break;
        case GLFW_KEY_RIGHT:
            rotation_vector -= entity_rotation_speed.y * (UP_VECTOR);
        break;
        default:
        break;
        }
    break;
    case KeyboardEvent::KEY_UP:
        switch (key_event->key) {
        case GLFW_KEY_W:
            direction_vector -= entity_speed.z * (FORWARD_VECTOR);
            break;
        case GLFW_KEY_S:
            direction_vector -= -entity_speed.z * (FORWARD_VECTOR);
            break;
        case GLFW_KEY_A:
            direction_vector -= -entity_speed.x * (RIGHT_VECTOR);
            break;
        case GLFW_KEY_D:
            direction_vector -= entity_speed.x * (RIGHT_VECTOR);
            break;
        case GLFW_KEY_UP:
            rotation_vector -= entity_rotation_speed.x * (RIGHT_VECTOR);
        break;
        case GLFW_KEY_DOWN:
            rotation_vector += entity_rotation_speed.x * (RIGHT_VECTOR);
        break;
        case GLFW_KEY_LEFT:
            rotation_vector -= entity_rotation_speed.y * (UP_VECTOR);
        break;
        case GLFW_KEY_RIGHT:
            rotation_vector += entity_rotation_speed.y * (UP_VECTOR);
        break;
        case GLFW_KEY_GRAVE_ACCENT:
            if (gravity_disabled) {
                gravity_disabled = false;
                TrillekGame::GetPhysicsSystem().SetNormalGravity(this->entity_id);
            }
            else {
                gravity_disabled = true;
                TrillekGame::GetPhysicsSystem().SetGravity(this->entity_id, zero_gravity);
            }
        break;
        default:
        break;
        }
    break;
    default:
    break;
    }
/*
    glm::vec3 move_vector = this->camera_transform->GetOrientation() * direction_vector;
    auto v_ptr = component::Create<Component::Velocity>(component::Velocity_type());
    component::Get<Component::Velocity>(v_ptr)->linear = glm::vec3(move_vector.x, move_vector.y, move_vector.z);
    glm::vec3 rotate_vector = this->camera_transform->GetOrientation() * rotation_vector;
    component::Get<Component::Velocity>(v_ptr)->angular = glm::vec3(rotate_vector.x, rotate_vector.y, rotate_vector.z);
    TrillekGame::GetPhysicsSystem().AddOrder(this->entity_id, std::move(v_ptr));
*/
}

} // graphics
} // trillek
