#include "systems/entity-system.hpp"
#include "components/renderable.hpp"
#include "trillek-game.hpp"

namespace trillek {

void EntityMap::RegisterTypes() {
    RegisterComponentType<graphics::Renderable>();
    RegisterSystem<graphics::Renderable>(&TrillekGame::GetGraphicSystem());
}

} // End of trillek
