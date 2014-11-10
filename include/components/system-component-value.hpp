#ifndef SYSTEM_COMPONENT_VALUE_HPP_INCLUDED
#define SYSTEM_COMPONENT_VALUE_HPP_INCLUDED

#include <map>
#include "component.hpp"
#include "bitmap.hpp"

namespace trillek { namespace component {

template<Component type,class T>
class SystemValueContainer {
public:
    typedef std::map<id_t, T,std::less<id_t>,
        TrillekAllocator<std::pair<const id_t,T>>> container_type;

    static container_type container;
    static BitMap<uint32_t> bitmap;
};

template<Component C, class T>
typename SystemValueContainer<C,T>::container_type SystemValueContainer<C,T>::container;

template<Component C, class T>
BitMap<uint32_t> SystemValueContainer<C,T>::bitmap;

template<Component C>
class SystemValueContainer<C,bool> {
public:
    typedef BitMap<uint32_t> container_type;

    static container_type container;
    static BitMap<uint32_t>& bitmap;
};

template<Component C>
typename SystemValueContainer<C,bool>::container_type SystemValueContainer<C,bool>::container;

template<Component C>
BitMap<uint32_t>& SystemValueContainer<C,bool>::bitmap = SystemValueContainer<C,bool>::container;

class SystemValue {
public:
    SystemValue() {};
    virtual ~SystemValue() {};

    template<Component type>
    typename type_trait<type>::value_type& Get(id_t entity_id, typename std::enable_if<!std::is_same<typename type_trait<type>::value_type,bool>::value>::type* = 0) {
        return Map<type>().at(entity_id);
    }

    // bool specialization
    template<Component type>
    typename type_trait<type>::value_type Get(id_t entity_id, typename std::enable_if<std::is_same<typename type_trait<type>::value_type,bool>::value>::type* = 0) {
        return Map<type>().at(entity_id);
    }

    template<Component C>
    bool Has(id_t entity_id) {
        return Bitmap<C>().at(entity_id);
    }

    template<Component C, class V>
    void Insert(id_t entity_id, V&& value, typename std::enable_if<!std::is_same<typename type_trait<C>::value_type,bool>::value>::type* = 0) {
        Update<C>(entity_id, std::forward<V>(value));
        SystemValueContainer<C,typename type_trait<C>::value_type>::bitmap[entity_id] = true;
    }

    // bool specialization
    template<Component C, class V>
    void Insert(id_t entity_id, V&& value, typename std::enable_if<std::is_same<typename type_trait<C>::value_type,bool>::value>::type* = 0) {
        Update<C>(entity_id, std::forward<V>(value));
    }

    template<Component type, class V>
    void Update(id_t entity_id, V&& value) {
        (Map<type>())[entity_id] = std::forward<V>(value);
    }

    template<Component type>
    void Remove(id_t entity_id, typename std::enable_if<!std::is_same<typename type_trait<type>::value_type,bool>::value>::type* = 0) {
        Map<type>().erase(entity_id);
        SystemValueContainer<type,typename type_trait<type>::value_type>::bitmap[entity_id] = false;
    }

    // bool specialization
    template<Component type>
    void Remove(id_t entity_id, typename std::enable_if<std::is_same<typename type_trait<type>::value_type,bool>::value>::type* = 0) {
        Map<type>().erase(entity_id);
    }

    template<Component C>
    typename SystemValueContainer<C,typename type_trait<C>::value_type>::container_type& Map() {
        return SystemValueContainer<C,typename type_trait<C>::value_type>::container;
    }

    template<Component C>
    const BitMap<uint32_t>& Bitmap() {
        return SystemValueContainer<C,typename type_trait<C>::value_type>::bitmap;
    }

};

} // namespace component
} // namespace trillek



#endif // SYSTEM_COMPONENT_VALUE_HPP_INCLUDED
