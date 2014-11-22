#include "trillek-game.hpp"
#include "trillek-scheduler.hpp"
#include "os.hpp"
#include "systems/fake-system.hpp"
#include "systems/physics.hpp"
#include "systems/meta-engine-system.hpp"
#include "systems/sound-system.hpp"
#include "systems/graphics.hpp"
#include "components/shared-component.hpp"
#include "components/system-component.hpp"
#include "components/system-component-value.hpp"

namespace trillek {

void TrillekGame::Initialize() {
    scheduler.reset(new TrillekScheduler);
    fake_system.reset(new FakeSystem);
    phys_sys.reset(new physics::PhysicsSystem);
    glfw_os.reset(new OS);
    shared_component.reset(new component::Shared);
    system_component.reset(new component::System);
    system_value_component.reset(new component::SystemValue);
    close_window = false;
    engine_sys.reset(new MetaEngineSystem);
}
std::unique_ptr<TrillekScheduler> TrillekGame::scheduler;
std::unique_ptr<FakeSystem> TrillekGame::fake_system;
std::unique_ptr<physics::PhysicsSystem> TrillekGame::phys_sys;
std::unique_ptr<OS> TrillekGame::glfw_os;
std::unique_ptr<component::Shared> TrillekGame::shared_component;
std::unique_ptr<component::System> TrillekGame::system_component;
std::unique_ptr<component::SystemValue> TrillekGame::system_value_component;
bool TrillekGame::close_window;

sound::System& TrillekGame::GetSoundSystem() {
    return *sound::System::GetInstance();
}

std::once_flag TrillekGame::once_graphics;
std::shared_ptr<graphics::RenderSystem> TrillekGame::gl_sys_ptr;
script::LuaSystem TrillekGame::lua_sys;
std::unique_ptr<MetaEngineSystem> TrillekGame::engine_sys;

graphics::RenderSystem& TrillekGame::GetGraphicSystem() {
    return *GetGraphicsInstance().get();
}

std::shared_ptr<graphics::RenderSystem> TrillekGame::GetGraphicsInstance() {
    std::call_once(TrillekGame::once_graphics,
        [ ] () {
            TrillekGame::gl_sys_ptr.reset(new graphics::RenderSystem());
            TrillekGame::gl_sys_ptr->RegisterTypes();
    });
    return std::shared_ptr<graphics::RenderSystem>(gl_sys_ptr);
}

} // End of namespace trillek
