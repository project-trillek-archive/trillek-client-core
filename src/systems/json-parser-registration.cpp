#include "systems/json-parser.hpp"

#include "systems/resource-system.hpp"
#include "systems/transform-system.hpp"
#include "systems/entity-system.hpp"
#include "trillek-game.hpp"

namespace trillek {
namespace json {

void System::RegisterTypes() {
    RegisterSerializer(TrillekGame::GetGraphicsInstance());
    RegisterSerializer(resource::ResourceMap::GetInstance());
    RegisterSerializer(transform::TransformMap::GetInstance());
    RegisterSerializer(EntityMap::GetInstance());
}

} // End of json
} // End of trillek
