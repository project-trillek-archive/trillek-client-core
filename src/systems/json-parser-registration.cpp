#include "systems/json-parser.hpp"

#include "systems/resource-system.hpp"
#include "systems/transform-system.hpp"

namespace trillek {
namespace json {

void System::RegisterTypes() {
    RegisterSerializer(resource::System::GetInstance());
    RegisterSerializer(transform::System::GetInstance());
}

} // End of json
} // End of trillek
