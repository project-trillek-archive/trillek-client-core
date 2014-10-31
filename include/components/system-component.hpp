#ifndef SYSTEM_COMPONENT_HPP_INCLUDED
#define SYSTEM_COMPONENT_HPP_INCLUDED

#include <map>
#include "component.hpp"
#include "container.hpp"
#include "bitmap.hpp"

namespace trillek { namespace component {

template<Component type>
class SystemContainer {
public:
    typedef std::map<id_t, std::shared_ptr<Container>,std::less<id_t>,
        TrillekAllocator<std::pair<const id_t,std::shared_ptr<Container>>>> container_type;

    static container_type container;
    static BitMap<uint32_t> bitmap;
};

template<Component type>
typename SystemContainer<type>::container_type SystemContainer<type>::container;

template<Component C>
BitMap<uint32_t> SystemContainer<C>::bitmap;

class System {
public:
    System() {};
    virtual ~System() {};

    template<Component type>
    typename type_trait<type>::value_type& Get(id_t entity_id) {
        return Map<type>().at(entity_id)->Get<typename type_trait<type>::value_type>();
    }

    template<Component type>
    std::shared_ptr<typename type_trait<type>::value_type> GetSharedPtr(id_t entity_id) {
        const auto& ptr = Map<type>().at(entity_id);
        return Container::GetSharedPtr<typename type_trait<type>::value_type>(ptr);
    }

    template<Component C>
    bool Has(id_t entity_id) {
        return Bitmap<C>().at(entity_id);
    }

    template<Component type, class V>
    void Insert(id_t entity_id, V&& value) {
        Map<type>().insert(std::make_pair(std::move(entity_id), std::allocate_shared<Container>(TrillekAllocator<Container>(), std::forward<V>(value))));
        LOGMSG(DEBUG) << "system inserting component " << reflection::GetTypeName<std::integral_constant<Component,type>>() << " for entity #" << entity_id;
        Bitmap<type>()[entity_id] = true;
    }

    template<Component type, class V>
    void Update(id_t entity_id, V&& value) {
        Map<type>().at(entity_id) = std::allocate_shared<Container>(TrillekAllocator<Container>(), std::forward<V>(value));
    }

    template<Component type>
    void Remove(id_t entity_id) {
        Map<type>().erase(entity_id);
        Bitmap<type>().at(entity_id) = false;
    }

    template<Component C>
    typename SystemContainer<C>::container_type& Map() {
        return SystemContainer<C>::container;
    }

    template<Component C>
    BitMap<uint32_t>& Bitmap() {
        return SystemContainer<C>::bitmap;
    }
};

} // namespace component
} // namespace trillek


#endif // SYSTEM_COMPONENT_HPP_INCLUDED
