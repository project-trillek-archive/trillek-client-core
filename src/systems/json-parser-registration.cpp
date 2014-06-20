#include "systems/json-parser.hpp"

#include "systems/resource-system.hpp"
#include "systems/transform-system.hpp"
#include "systems/sound-system.hpp"

namespace trillek {
namespace json {

void System::RegisterTypes() {
    RegisterSerializer(resource::System::GetInstance());
    RegisterSerializer(transform::System::GetInstance());
    RegisterSerializer(sound::System::GetInstance());
}

} // End of json
} // End of trillek
