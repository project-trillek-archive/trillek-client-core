#ifndef SHARED_COMPONENT_HPP_INCLUDED
#define SHARED_COMPONENT_HPP_INCLUDED

#include "systems/rewindable-map.hpp"
#include "component.hpp"
#include "component-container.hpp"

namespace trillek { namespace component {

template<Component C,class T>
class SharedContainer {
public:
    static RewindableMap<id_t, std::shared_ptr<const Container>,frame_tp,30> container;
};

template<Component C,class T>
RewindableMap<id_t, std::shared_ptr<const Container>,frame_tp,30> SharedContainer<C,T>::container;

template<Component C>
class SharedContainer<C,bool> {
public:
    static RewindableMap<id_t, bool,frame_tp,30> container;
};

template<Component C>
RewindableMap<id_t, bool,frame_tp,30> SharedContainer<C,bool>::container;

class Shared {
public:
    Shared() {};
    virtual ~Shared() {};

    template<Component type>
    const typename type_trait<type>::value_type& Get(id_t entity_id) {
        return *component::Get<type>(Map<type>().Map().at(entity_id));
    }

    template<Component type>
    std::shared_ptr<const Container> GetConstContainer(id_t entity_id) {
        return Map<type>().Map().at(entity_id);
    }

    template<Component type>
    std::shared_ptr<const typename type_trait<type>::value_type> GetSharedPtr(id_t entity_id) {
        const auto& ptr = std::const_pointer_cast<Container>(Map<type>().Map().at(entity_id));
        return component::Get<type,const typename type_trait<type>::value_type>(ptr);
    }

    template<Component type>
    bool Has(id_t entity_id) {
        return Map<type>().Map().count(entity_id);
    }

    template<Component type, class V>
    void Insert(id_t entity_id, V&& value, typename std::enable_if<!util::is_shared_ptr<typename std::decay<V>::type>::value>::type* = 0) {
        Map<type>().Insert(entity_id, component::CreateConst<type>(std::forward<V>(value)));
    }


    template<Component type, class V>
    void Insert(id_t entity_id, V&& value, typename std::enable_if<util::is_shared_ptr<typename std::decay<V>::type>::value>::type* = 0) {
        Map<type>().Insert(entity_id, std::forward<V>(value));
    }

    template<Component type, class V>
    void Update(id_t entity_id, V&& value, typename std::enable_if<!util::is_shared_ptr<typename std::decay<V>::type>::value>::type* = 0) {
        Map<type>().Update(entity_id, component::CreateConst<type>(std::forward<V>(value)));
    }

    template<Component type, class V>
    void Update(id_t entity_id, V&& value, typename std::enable_if<util::is_shared_ptr<typename std::decay<V>::type>::value>::type* = 0) {
        Map<type>().Update(entity_id, std::forward<V>(value));
    }

    template<Component type>
    void Remove(id_t entity_id) {
        Map<type>().Remove(entity_id);
    }


    template<Component C>
    void Commit(frame_tp frame) {
        Map<C>().Commit(frame);
    }

    template<Component C>
    const SharedContainerConst<id_t,typename type_trait<C>::value_type>& GetLastPositiveCommit() {
        return Map<C>().GetLastPositiveCommit();
    }

    template<Component C>
    const BitMap<uint32_t>& GetLastPositiveBitMap() {
        return Map<C>().GetLastPositiveBitMap();
    }

    template<Component C>
    const BitMap<uint32_t>& Bitmap() {
        return Map<C>().Bitmap();
    }

    template<Component C>
    RewindableMap<id_t, std::shared_ptr<const Container>,frame_tp,30>& Map() {
        return SharedContainer<C,typename type_trait<C>::value_type>::container;
    }
};

} // namespace component
} // namespace trillek

#endif // SHARED_COMPONENT_HPP_INCLUDED
