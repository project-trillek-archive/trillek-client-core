#include "trillek-game.hpp"
#include "graphics/renderable.hpp"
#include "graphics/shader.hpp"
#include "graphics/light.hpp"
#include "resources/md5mesh.hpp"
#include "resources/md5anim.hpp"
#include "resources/pixel-buffer.hpp"
#include "resources/text-file.hpp"

#include "systems/resource-system.hpp"
#include "systems/transform-system.hpp"
#include "systems/component-factory.hpp"
#include "util/json-parser.hpp"

namespace trillek {

void ComponentFactory::RegisterTypes() {
    RegisterComponentType<graphics::Renderable>();
    RegisterComponentType<graphics::LightBase>();
    RegisterSystem<graphics::Renderable>(&TrillekGame::GetGraphicSystem());
    RegisterSystem<graphics::LightBase>(&TrillekGame::GetGraphicSystem());
}

void util::JSONPasrser::RegisterTypes() {
    RegisterParser(TrillekGame::GetGraphicsInstance());
    RegisterParser(resource::ResourceMap::GetInstance());
    RegisterParser(TransformMap::GetInstance());
    RegisterParser(ComponentFactory::GetInstance());
}

void resource::ResourceMap::RegisterTypes() {
    RegisterResourceType<resource::TextFile>();
    RegisterResourceType<resource::MD5Mesh>();
    RegisterResourceType<resource::PixelBuffer>();
    RegisterResourceType<resource::MD5Anim>();
}

} // End of trillek
