#ifndef SIX_DOF_CAMERA_HPP_INCLUDED
#define SIX_DOF_CAMERA_HPP_INCLUDED

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "graphics/camera.hpp"
#include "trillek-game.hpp"

namespace trillek {
namespace graphics {

// This is a 6 degree-of-freedom camera allowing movement on any axis relative to the current orientation.
class SixDOFCamera :
    public CameraBase {
public:
    /**
     * \brief Computes the view matrix for a 6 DOF camera using the provided transform.
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
private:
};

} // End of graphics

namespace reflection {
TRILLEK_MAKE_IDTYPE_NAME(graphics::SixDOFCamera, "camera", 2002)
} // namespace reflection

} // End of trillek
#endif
