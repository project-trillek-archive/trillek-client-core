#include "components/component.hpp"
#include "property.hpp"
#include "container.hpp"
#include "physics/collidable.hpp"
#include "trillek-game.hpp"
#include "components/system-component-value.hpp"
#include "components/system-component.hpp"
#include "components/shared-component.hpp"

namespace trillek { namespace component {

template<>
struct ContainerRef<System> {
    static System& container;
};

System& ContainerRef<System>::container = TrillekGame::GetSystemComponent();

template<>
struct ContainerRef<SystemValue> {
    static SystemValue& container;
};

SystemValue& ContainerRef<SystemValue>::container = TrillekGame::GetSystemValueComponent();

template<>
struct ContainerRef<Shared> {
    static Shared& container;
};

Shared& ContainerRef<Shared>::container = TrillekGame::GetSharedComponent();

template<>
std::shared_ptr<Container> Initialize<Component::VelocityMax>(const std::vector<Property> &properties) {
    glm::vec3 lmax(0.0f,0.0f,0.0f), amax(0.0f,0.0f,0.0f);
    id_t entity_id;
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if (name == "max_horizontal") {
            auto tlmax = (float) p.Get<double>();
            lmax.x = tlmax;
            lmax.z = tlmax;
        }
        else if (name == "max_vertical") {
            lmax.y = (float) p.Get<double>();
        }
        else if (name == "max_angular") {
            auto tamax = (float) p.Get<double>();
            amax.x = tamax;
            amax.y = tamax;
            amax.z = tamax;
        }
        else if (name == "entity_id") {
            entity_id = p.Get<id_t>();
        }
        else {
            LOGMSG(ERROR) << "VelocityMax: Unknown property: " << name;
            return nullptr;
        }
    }
    return std::allocate_shared<Container>(TrillekAllocator<Container>(), VelocityMax_type(std::move(lmax), std::move(amax)));
}

template<>
id_t Initialize<Component::ReferenceFrame>(bool& result, const std::vector<Property> &properties) {
    id_t entity_id, reference_id;
    Velocity_type ref_velocity;
    result = false;
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if (name == "entity") {
            reference_id = p.Get<id_t>();
            if (! TrillekGame::GetSharedComponent().Has<Component::Velocity>(reference_id)) {
                LOGMSG(ERROR) << "ReferenceFrame: entity #" << reference_id << "does not have velocity";
                return 0;
            }
            ref_velocity = TrillekGame::GetSharedComponent().Get<Component::Velocity>(reference_id);
            result = true;
        }
        else if (name == "entity_id") {
            entity_id = p.Get<id_t>();
        }
        else {
            LOGMSG(ERROR) << "ReferenceFrame: Unknown property: " << name;
        }
    }
    if (result) {
        // create IsReferenceFrame and CombinedVelocity components
        TrillekGame::GetSystemValueComponent().Insert<Component::IsReferenceFrame>(reference_id, true);
        TrillekGame::GetSystemComponent().Insert<Component::CombinedVelocity>(entity_id, ref_velocity);
        return reference_id;
    }
    return 0;
}

template<>
std::shared_ptr<Container> Initialize<Component::Collidable>(const std::vector<Property> &properties) {
    auto ret = std::allocate_shared<Container>(TrillekAllocator<Container>(), physics::Collidable());
    if (ret->Get<physics::Collidable>().Initialize(properties)) {
        return std::move(ret);
    }
    return nullptr;
}

} // namespace component
} // namespace trillek