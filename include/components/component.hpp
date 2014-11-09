#ifndef COMPONENT_HPP_INCLUDED
#define COMPONENT_HPP_INCLUDED

#include <vector>
#include "systems/physics.hpp"
#include "bitmap.hpp"
#include "components/component-enum.hpp"
#include "components/component-container.hpp"

namespace trillek {

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

// A class to hold a container reference
template<class T>
struct ContainerRef {
    static T& container;
};

//template<Component C, class T=typename type_trait<C>::value_type>
//std::shared_ptr<Container> Create(const typename type_trait<C>::value_type& comp);

template<Component C>
std::shared_ptr<Container> Initialize(const std::vector<Property> &properties) {
    return component::Create<C>(typename type_trait<C>::value_type());
};

template<Component C>
typename type_trait<C>::value_type Initialize(bool& result, const std::vector<Property> &properties) {
    result = true;
    return typename type_trait<C>::value_type();
};

template<class T>
static void OnTrue(const BitMap<T>& bitmap, const std::function<void(id_t)>& operation) {
    // TODO replace 10000 by the number of entities in the game.
    auto end = bitmap.DefaultValue() ? std::max(bitmap.size(), size_t(10000)) : bitmap.size();
    for (auto i = bitmap.enumerator(10000); *i < end; ++i) {
        operation(*i);
    }
};

template<Component C>
static typename container_type_trait<C>::container_type GetContainer() {
    return ContainerRef<typename container_type_trait<C>::container_type>::container;
};

template<Component C>
static const typename type_trait<C>::value_type& Get(id_t entity_id, typename std::enable_if<!std::is_same<typename type_trait<C>::value_type,bool>::value>::type* = 0) {
    return GetContainer<C>().Get<C>(entity_id);
}

template<Component C>
static typename type_trait<C>::value_type Get(id_t entity_id, typename std::enable_if<std::is_same<typename type_trait<C>::value_type,bool>::value>::type* = 0) {
    return GetContainer<C>().Get<C>(entity_id);
}

template<Component C>
std::shared_ptr<typename type_trait<C>::value_type> GetSharedPtr(id_t entity_id) {
    return GetContainer<C>().GetSharedPtr<C>(entity_id);
}

template<Component C>
std::shared_ptr<const typename type_trait<C>::value_type> GetConstSharedPtr(id_t entity_id) {
    return GetContainer<C>().GetSharedPtr<C>(entity_id);
}

template<Component C, class V>
static void Insert(id_t entity_id, V&& value) {
    GetContainer<C>().Insert<C>(entity_id, std::forward<V>(value));
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
static bool Has(id_t entity_id) {
    return GetContainer<C>().Has<C>(entity_id);
}

template<Component C>
static const BitMap<uint32_t>& Bitmap() {
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
} // namespace trillek

#endif
