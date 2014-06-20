#ifndef LIGHT_HPP_INCLUDED
#define LIGHT_HPP_INCLUDED

#include <memory>
#include <vector>
#include "systems/component-factory.hpp"

namespace trillek {
namespace graphics {

/**
 * \brief The base light component acts as a simple point light in the scene
 */
class LightBase : public ComponentBase {
public:

    LightBase() { }
    virtual ~LightBase() { }

    /**
     * \brief Initializes the light component with the provided properties
     *
     * \param[in] const std::vector<Property>& properties The creation properties for the component.
     * \return bool True if initialization finished with no errors.
     */
    bool Initialize(const std::vector<Property> &properties);

private:
};

} // namespace graphics

namespace reflection {

template <> inline const char* GetTypeName<graphics::LightBase>() { return "light"; }
template <> inline const unsigned int GetTypeID<graphics::LightBase>() { return 2001; }

} // End of reflection
} // namespace trillek

#endif
