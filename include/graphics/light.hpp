#ifndef LIGHT_HPP_INCLUDED
#define LIGHT_HPP_INCLUDED

#include "opengl.hpp"
#include "type-id.hpp"
#include <memory>
#include <vector>
#include "components/component-factory.hpp"

namespace trillek {
namespace graphics {

/**
 * \brief The base light component acts as a simple point light in the scene
 */
class LightBase : public ComponentBase {
public:

    LightBase() {
        enabled = true;
        shadows = false;
        lighttype = 0;
    }
    virtual ~LightBase() { }

    /**
     * \brief Initializes the light component with the provided properties
     *
     * \param[in] const std::vector<Property>& properties The creation properties for the component.
     * \return bool True if initialization finished with no errors.
     */
    virtual bool Initialize(const std::vector<Property> &properties);

    bool enabled;
    bool shadows;
    GLuint lighttype;
    glm::vec3 color;
    glm::mat4x4 depthmatrix;
    std::vector<Property> light_props;
};

} // namespace graphics

namespace reflection {
TRILLEK_MAKE_IDTYPE_NAME(graphics::LightBase, "light", 2001)
} // End of reflection

} // namespace trillek

#endif
