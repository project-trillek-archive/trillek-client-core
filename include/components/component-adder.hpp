#ifndef COMPONENT_ADDER_HPP_INCLUDED
#define COMPONENT_ADDER_HPP_INCLUDED

#include "shared-component.hpp"
#include "system-component.hpp"
#include "system-component-value.hpp"

namespace trillek {

using namespace component;

template<Component type>
static std::shared_ptr<Container> CreateComponent(id_t entity_id,
                                const std::vector<Property> &properties) {
    auto type_id = static_cast<uint32_t>(type);
    auto sharedcomp = component::Initialize<type>(properties);
    if (!sharedcomp) {
        LOGMSG(ERROR) << "Error while initializing component "
                        << reflection::GetTypeName<std::integral_constant<Component,type>>() << " for entity id #" << entity_id;
        return nullptr;
    }
    return std::move(sharedcomp);
}


template<ComponentType cptype,Component type,class T = typename type_trait<type>::value_type>
class ComponentAdder;

template<Component C>
class ComponentAdder<DYNAMIC,C> {
public:
    ComponentAdder(SystemBase* system) : system(*system) {};

    bool Create(id_t entity_id, const std::vector<Property> &properties) {
        auto comp = CreateComponent<C>(entity_id, properties);

        if (comp) {
            system.AddDynamicComponent
                    (entity_id,std::move(comp));
            return true;
        }
        return false;
    }
private:
    SystemBase& system;
};

template<Component C,class T>
class ComponentAdder<SYSTEM,C,T> {
public:
    ComponentAdder(component::System& system) : system(system) {};

    bool Create(id_t entity_id, const std::vector<Property> &properties) {
        auto comp = CreateComponent<C>(entity_id, properties);

        if (comp) {
            LOGMSG(DEBUG) << "Adding system component " << reflection::GetTypeName<std::integral_constant<Component,C>>() << " to entity #" << entity_id;
            system.Map<C>().insert(std::make_pair(std::move(entity_id), std::move(comp)));
            return true;
        }
        return false;
    }
private:
    component::System& system;
};

template<Component type>
class ComponentAdder<SYSTEM,type,bool> {
public:
    ComponentAdder(component::SystemValue& system) : system(system) {};

    bool Create(id_t entity_id, const std::vector<Property> &properties) {
        bool result = false;
        auto comp = component::Initialize<type>(result, properties);
        if (!result) {
            LOGMSGC(ERROR) << "Error while initializing component "
                            << reflection::GetTypeName<std::integral_constant<Component,type>>() << " for entity id #" << entity_id;
            return false;
        }

        LOGMSG(DEBUG) << "Adding system component " << reflection::GetTypeName<std::integral_constant<Component,type>>() << " to entity #" << entity_id;
        system.Insert<type>(entity_id, std::move(comp));
        return true;
    }

private:
    component::SystemValue& system;
};

template<Component type>
class ComponentAdder<SYSTEM,type,uint32_t> {
public:
    ComponentAdder(component::SystemValue& system) : system(system) {};

    bool Create(id_t entity_id, const std::vector<Property> &properties) {
        bool result = false;
        auto comp = component::Initialize<type>(result, properties);
        if (!result) {
            LOGMSGC(ERROR) << "Error while initializing component "
                            << reflection::GetTypeName<std::integral_constant<Component,type>>() << " for entity id #" << entity_id;
            return false;
        }

        LOGMSG(DEBUG) << "Adding system component " << reflection::GetTypeName<std::integral_constant<Component,type>>() << " to entity #" << entity_id;
        system.Insert<type>(entity_id, std::move(comp));
        return true;
    }

private:
    component::SystemValue& system;
};

template<component::Component C>
class ComponentAdder<SHARED,C> {
public:
    ComponentAdder(component::Shared& shared) : shared(shared) {};

    bool Create(id_t entity_id, const std::vector<Property> &properties) {
        auto comp = CreateComponent<C>(entity_id, properties);

        if (comp) {
            LOGMSG(DEBUG) << "Adding shared component " << reflection::GetTypeName<std::integral_constant<Component,C>>() << " to entity #" << entity_id;
            shared.Map<C>().Insert(entity_id, std::move(comp));
            return true;
        }
        return false;
    }
private:
    component::Shared& shared;
};

}

#endif // COMPONENT_ADDER_HPP_INCLUDED
