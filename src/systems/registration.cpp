#include "trillek-game.hpp"
#include "graphics/renderable.hpp"
#include "graphics/camera.hpp"
#include "graphics/six-dof-camera.hpp"
#include "graphics/shader.hpp"
#include "graphics/light.hpp"
#include "graphics/render-layer.hpp"
#include "graphics/render-list.hpp"
#include "systems/graphics.hpp"
#include "systems/sound-system.hpp"
#include "resources/md5anim.hpp"
#include "resources/pixel-buffer.hpp"

#include "components/component-factory.hpp"
#include "components/component-templates.hpp"
#include "physics/collidable.hpp"
#include "resources/md5mesh.hpp"
#include "resources/md5anim.hpp"
#include "resources/obj.hpp"
#include "resources/pixel-buffer.hpp"
#include "resources/text-file.hpp"

#include "systems/physics.hpp"
#include "systems/resource-system.hpp"
#include "systems/transform-system.hpp"
#include "util/json-parser.hpp"

namespace trillek {
/** \brief Type of component
 *
 * DYNAMIC: The component is passed to the system and then stored in the
 * ComponentFactory container
 * SYSTEM : The component is passed to SystemComponent and is stored there
 * SHARED : The component is passed to SharedComponent and is stored there
 *
 * system_value is like system, but for primitive values that don't need a pointer
 * to be passed as argument of a fonction (bool, uint32_t).
 *
 * Only SHARED components can be shared between systems in different threads.
 * By default, DYNAMIC components are registered.
 *
 * The preferred choice is SYSTEM.
 */

using component::Component;

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
    auto& shared = TrillekGame::GetSharedComponent();
    auto& system = TrillekGame::GetSystemComponent();
    auto& system_value = TrillekGame::GetSystemValueComponent();
    RegisterComponentType(ComponentAdder<SYSTEM,Component::Collidable>(system));
    RegisterComponentType(ComponentAdder<SHARED,Component::Velocity, bool>(shared));
    RegisterComponentType(ComponentAdder<SHARED,Component::VelocityMax, bool>(shared));
    RegisterComponentType(ComponentAdder<SYSTEM,Component::ReferenceFrame,id_t>(system_value));
    RegisterComponentType(ComponentAdder<SYSTEM,Component::IsReferenceFrame,bool>(system_value));
    RegisterComponentType(ComponentAdder<SYSTEM,Component::CombinedVelocity>(system));
    RegisterComponentType(ComponentAdder<SYSTEM,Component::ReferenceFrame,id_t>(system_value));
    RegisterComponentType(ComponentAdder<SYSTEM,Component::OxygenRate,float_t>(system_value));
    RegisterComponentType(ComponentAdder<SYSTEM,Component::Health,uint32_t>(system_value));
    RegisterComponentType(ComponentAdder<SHARED,Component::GraphicTransform,bool>(shared));
    RegisterComponentType(ComponentAdder<SHARED,Component::GameTransform,bool>(shared));
    RegisterComponentType<graphics::Renderable>();
    RegisterComponentType<graphics::LightBase>();
    RegisterComponentType<graphics::SixDOFCamera>();
    RegisterSystem<graphics::Renderable>(&TrillekGame::GetGraphicSystem());
    RegisterSystem<graphics::LightBase>(&TrillekGame::GetGraphicSystem());
    RegisterSystem<graphics::CameraBase>(&TrillekGame::GetGraphicSystem());
}

void util::JSONPasrser::RegisterTypes() {
    RegisterParser(TrillekGame::GetGraphicsInstance());
    RegisterParser(sound::System::GetInstance());
    RegisterParser(resource::ResourceMap::GetInstance());
    RegisterParser(TransformMap::GetInstance());
    RegisterParser(ComponentFactory::GetInstance());
}

void resource::ResourceMap::RegisterTypes() {
    RegisterResourceType<resource::TextFile>();
    RegisterResourceType<resource::MD5Mesh>();
    RegisterResourceType<resource::OBJ>();
    RegisterResourceType<resource::PixelBuffer>();
    RegisterResourceType<resource::MD5Anim>();
}

} // End of trillek
