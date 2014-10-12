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
     */
    glm::mat4 GetViewMatrix();

    /**
     * \brief Handles keyboard events.
     *
     * This method calls the physics system SetForce method after each event.
     */
    void Notify(const KeyboardEvent* key_event);
private:
};

} // End of graphics

namespace reflection {
TRILLEK_MAKE_IDTYPE_NAME(graphics::SixDOFCamera, "camera", 2002)
} // namespace reflection

} // End of trillek
#endif
