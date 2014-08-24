#ifndef COMPONENT_TEMPLATES_HPP_INCLUDED
#define COMPONENT_TEMPLATES_HPP_INCLUDED

#include "component.hpp"

namespace trillek {

class Container;

namespace component {

extern template std::shared_ptr<Container> Initialize<Component::VelocityMax>(const std::vector<Property> &properties);
extern template std::shared_ptr<Container> Initialize<Component::Collidable>(const std::vector<Property> &properties);
extern template std::shared_ptr<Container> Initialize<Component::ReferenceFrame>(const std::vector<Property> &properties);

} // namespace component
} // namespace trillek

#endif // COMPONENT_TEMPLATES_HPP_INCLUDED
