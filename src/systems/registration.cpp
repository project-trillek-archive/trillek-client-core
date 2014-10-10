#include "trillek-game.hpp"

#if defined(_CLIENT_) || defined(_STANDALONE_)
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
#endif

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
 * SYSTEM : The component is passed to the system that must store it.
 * SHARED : The component is passed to SharedComponent and is stored there
 *
 * Only SHARED components can be shared between systems in different threads.
 * By default, DYNAMIC components are registered.
 */

using component::Component;

#if defined(_CLIENT_) || defined(_STANDALONE_)
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
#endif

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
    RegisterComponentType(ComponentAdder<SHARED,Component::Transform, bool>(shared));
#if defined(_CLIENT_) || defined(_STANDALONE_)
    RegisterComponentType<graphics::Renderable>();
    RegisterComponentType<graphics::LightBase>();
    RegisterComponentType<graphics::SixDOFCamera>();
    RegisterSystem<graphics::Renderable>(&TrillekGame::GetGraphicSystem());
    RegisterSystem<graphics::LightBase>(&TrillekGame::GetGraphicSystem());
    RegisterSystem<graphics::CameraBase>(&TrillekGame::GetGraphicSystem());
#endif
}

void util::JSONPasrser::RegisterTypes() {
#if defined(_CLIENT_) || defined(_STANDALONE_)
    RegisterParser(TrillekGame::GetGraphicsInstance());
    RegisterParser(sound::System::GetInstance());
#endif
    RegisterParser(resource::ResourceMap::GetInstance());
    RegisterParser(TransformMap::GetInstance());
    RegisterParser(ComponentFactory::GetInstance());
}

void resource::ResourceMap::RegisterTypes() {
    RegisterResourceType<resource::TextFile>();
    RegisterResourceType<resource::MD5Mesh>();
    RegisterResourceType<resource::OBJ>();
#if defined(_CLIENT_) || defined(_STANDALONE_)
    RegisterResourceType<resource::PixelBuffer>();
    RegisterResourceType<resource::MD5Anim>();
#endif
}

} // End of trillek
