#include "trillek-game.hpp"

#include "graphics/renderable.hpp"
#include "graphics/camera.hpp"
#include "graphics/six-dof-camera.hpp"
#include "graphics/shader.hpp"
#include "graphics/light.hpp"
#include "graphics/render-layer.hpp"
#include "graphics/render-list.hpp"

#include "physics/collidable.hpp"

#include "resources/md5mesh.hpp"
#include "resources/md5anim.hpp"
#include "resources/obj.hpp"
#include "resources/pixel-buffer.hpp"
#include "resources/text-file.hpp"

#include "systems/resource-system.hpp"
#include "systems/graphics.hpp"
#include "systems/transform-system.hpp"
#include "systems/component-factory.hpp"
#include "systems/sound-system.hpp"
#include "util/json-parser.hpp"

namespace trillek {

void graphics::RenderSystem::RegisterTypes() {
    // textures are implicitly instanced most of the time
    RegisterClassGenParser<graphics::Texture>();
    RegisterClassGenParser<graphics::Shader>();
    RegisterClassGenParser<graphics::RenderAttachment>();
    RegisterClassGenParser<graphics::RenderLayer>();
    RegisterClassGenParser<graphics::RenderList>();
    RegisterStaticParsers();
    RegisterListResolvers();
}

void ComponentFactory::RegisterTypes() {
    RegisterComponentType<graphics::Renderable>();
    RegisterComponentType<graphics::LightBase>();
    RegisterComponentType<graphics::SixDOFCamera>();
    RegisterComponentType<physics::Collidable>();
    RegisterSystem<graphics::Renderable>(&TrillekGame::GetGraphicSystem());
    RegisterSystem<physics::Collidable>(&TrillekGame::GetPhysicsSystem());
    RegisterSystem<graphics::LightBase>(&TrillekGame::GetGraphicSystem());
    RegisterSystem<graphics::CameraBase>(&TrillekGame::GetGraphicSystem());
}

void util::JSONPasrser::RegisterTypes() {
    RegisterParser(TrillekGame::GetGraphicsInstance());
    RegisterParser(resource::ResourceMap::GetInstance());
    RegisterParser(TransformMap::GetInstance());
    RegisterParser(ComponentFactory::GetInstance());
    RegisterParser(sound::System::GetInstance());
}

void resource::ResourceMap::RegisterTypes() {
    RegisterResourceType<resource::TextFile>();
    RegisterResourceType<resource::MD5Mesh>();
    RegisterResourceType<resource::OBJ>();
    RegisterResourceType<resource::PixelBuffer>();
    RegisterResourceType<resource::MD5Anim>();
}

} // End of trillek
