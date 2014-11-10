#ifndef COMPONENT_ENUM_HPP_INCLUDED
#define COMPONENT_ENUM_HPP_INCLUDED

#define TRILLEK_MAKE_COMPONENT(enumerator,name,type,container) \
    namespace component {\
    template<> struct type_trait<Component::enumerator> { typedef type value_type; };\
    typedef type enumerator##_type;\
    \
    template<> struct container_type_trait<Component::enumerator> { typedef container container_type; };\
    }\
    \
    namespace reflection {\
    template <>\
    inline const char* GetTypeName<std::integral_constant<component::Component,component::Component::enumerator>>()\
                { return name; };\
    template <>\
    inline unsigned int GetTypeID<std::integral_constant<component::Component,component::Component::enumerator>>()\
                { return static_cast<uint32_t>(component::Component::enumerator); };\
    }

namespace trillek {

class Property;
class Transform;

namespace physics {
class Collidable;
}

namespace component {

class Container;
class System;
class Shared;
class SystemValue;

enum class Component : uint32_t {
    Velocity = 1,               // instant displacement
    VelocityMax,                // maximum instant displacement
    ReferenceFrame,             // id of the parent entity for the transform
    IsReferenceFrame,           // true if this entity has at least a child
    CombinedVelocity,           // ?
    Collidable,                 // wrapper object for body TODO:make a true component
    OxygenRate,                 // Oxygen rate associated with an entity (room...)
    Health,                     // Health level
    Immune,                     // true = Immune
    GraphicTransform,           // transform used to display the entity
    GameTransform               // last confirmed transform
};

template<Component C> struct type_trait;
template<Component C> struct container_type_trait;

} // namespace component

TRILLEK_MAKE_COMPONENT(Collidable,"collidable",trillek::physics::Collidable,System)
TRILLEK_MAKE_COMPONENT(Velocity,"velocity",trillek::physics::VelocityStruct,Shared)
TRILLEK_MAKE_COMPONENT(VelocityMax,"velocity-max",trillek::physics::VelocityMaxStruct,Shared)
TRILLEK_MAKE_COMPONENT(ReferenceFrame,"reference-frame",id_t,SystemValue)
TRILLEK_MAKE_COMPONENT(IsReferenceFrame,"is-reference-frame",bool,SystemValue)
TRILLEK_MAKE_COMPONENT(CombinedVelocity,"combined-velocity",trillek::physics::VelocityStruct,System)
TRILLEK_MAKE_COMPONENT(OxygenRate,"oxygen-rate",float_t,SystemValue)
TRILLEK_MAKE_COMPONENT(Health,"health",uint32_t,SystemValue)
TRILLEK_MAKE_COMPONENT(Immune,"immune",bool,SystemValue)
TRILLEK_MAKE_COMPONENT(GraphicTransform,"graphic-transform",trillek::Transform, Shared)
TRILLEK_MAKE_COMPONENT(GameTransform,"game-transform",trillek::Transform, Shared)

} // namespace trillek

#endif // COMPONENT_ENUM_HPP_INCLUDED
