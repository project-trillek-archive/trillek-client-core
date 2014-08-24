#ifndef COMPONENT_HPP_INCLUDED
#define COMPONENT_HPP_INCLUDED

#include <vector>
#include "systems/physics.hpp"
#include "bitmap.hpp"

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
    inline constexpr const char* GetTypeName<std::integral_constant<component::Component,component::Component::enumerator>>()\
                { return name; };\
    template <>\
    inline constexpr unsigned int GetTypeID<std::integral_constant<component::Component,component::Component::enumerator>>()\
                { return static_cast<uint32_t>(component::Component::enumerator); };\
    }

namespace trillek {

class Property;
class Container;
class Transform;

namespace physics {
class Collidable;
}

class ComponentBase {
public:

    ComponentBase() : component_type_id(0) { }
    ~ComponentBase() { }

    /**
     * \brief Returns a component with the specified name.
     *
     * \param[in] const std::vector<Property> &properties The creation properties for the component.
     * \return bool True if initialization finished with no errors.
     */
    virtual bool Initialize(const std::vector<Property> &properties) { return false; };

    unsigned int component_type_id;
};

namespace component {

class System;
class Shared;
class SystemValue;

enum class Component : uint32_t {
    Velocity = 1,
    VelocityMax,
    ReferenceFrame,
    IsReferenceFrame,
    CombinedVelocity,
    Collidable,
    Transform
};

// A class to hold a container reference
template<class T>
struct ContainerRef {
    static T& container;
};

template<Component C> struct type_trait;
template<Component C> struct container_type_trait;

template<Component C>
std::shared_ptr<Container> Initialize(const std::vector<Property> &properties) {
    return std::allocate_shared<Container>(TrillekAllocator<Container>(), typename type_trait<C>::value_type());
};

template<Component C>
typename type_trait<C>::value_type Initialize(bool& result, const std::vector<Property> &properties) {
    result = true;
    return typename type_trait<C>::value_type();
};

template<class T>
static void OnTrue(const BitMap<T>& bitmap, const std::function<void(id_t)>& operation) {
    for (auto i = bitmap.enumerator(); *i < bitmap.size(); ++i) {
        operation(*i);
    }
};

template<Component C>
static typename container_type_trait<C>::container_type GetContainer() {
    return ContainerRef<typename container_type_trait<C>::container_type>::container;
};

template<Component C>
static const typename type_trait<C>::value_type& Get(id_t entity_id) {
    return GetContainer<C>().Get<C>(entity_id);
}

template<Component C, class V>
static void Update(id_t entity_id, V&& value) {
    GetContainer<C>().Update<C>(entity_id, std::forward<V>(value));
}

template<Component C>
static void Remove(id_t entity_id) {
    GetContainer<C>().Remove<C>(entity_id);
}

template<Component C>
static BitMap<uint32_t>& Bitmap() {
    return GetContainer<C>().Bitmap<C>();
};

template<Component C>
static void Commit(frame_tp frame) {
    GetContainer<C>().Commit<C>(frame);
}

template<Component C>
static const std::map<id_t,const typename type_trait<C>::value_type, std::less<id_t>,
            TrillekAllocator<std::pair<const id_t,typename type_trait<C>::value_type>>>&
                                                         GetLastPositiveCommit() {
    return GetContainer<C>().GetLastPositiveCommit<C>();
}

template<Component C>
static const BitMap<uint32_t>& GetLastPositiveBitMap() {
    return GetContainer<C>().GetLastPositiveBitMap<C>();
}
} // namespace component

TRILLEK_MAKE_COMPONENT(Collidable,"collidable",trillek::physics::Collidable,System)
TRILLEK_MAKE_COMPONENT(Velocity,"velocity",trillek::physics::VelocityStruct,Shared)
TRILLEK_MAKE_COMPONENT(VelocityMax,"velocity-max",trillek::physics::VelocityMaxStruct,Shared)
TRILLEK_MAKE_COMPONENT(ReferenceFrame,"reference-frame",id_t,SystemValue)
TRILLEK_MAKE_COMPONENT(IsReferenceFrame,"is-reference-frame",bool,SystemValue)
TRILLEK_MAKE_COMPONENT(CombinedVelocity,"combined-velocity",trillek::physics::VelocityStruct,System)
TRILLEK_MAKE_COMPONENT(Transform,"transform",trillek::Transform, Shared)

} // namespace trillek

#endif
