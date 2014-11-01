#ifndef COMPONENT_TEMPLATES_HPP_INCLUDED
#define COMPONENT_TEMPLATES_HPP_INCLUDED

#include "component.hpp"

namespace trillek {

namespace component {

class Container;

extern template std::shared_ptr<Container> Initialize<Component::VelocityMax>(const std::vector<Property> &properties);
extern template std::shared_ptr<Container> Initialize<Component::Collidable>(const std::vector<Property> &properties);
extern template id_t Initialize<Component::ReferenceFrame>(bool& result, const std::vector<Property> &properties);

} // namespace component
} // namespace trillek

#endif // COMPONENT_TEMPLATES_HPP_INCLUDED
