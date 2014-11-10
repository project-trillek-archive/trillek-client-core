#ifndef IORDER_HPP_INCLUDED
#define IORDER_HPP_INCLUDED

#include "components/component.hpp"

namespace trillek {
namespace usercommand {

    using namespace trillek::component;

    void Execute(id_t id, std::shared_ptr<component::Container>&& usercommand) {
        switch(usercommand->GetTypeId()) {
        case (static_cast<uint32_t>(Component::Velocity)) : {
            Update<Component::Velocity>(id, std::move(usercommand));
        }
        }
    }

} // namespace usercommand
} // namespace trillek

#endif // IORDER_HPP_INCLUDED
