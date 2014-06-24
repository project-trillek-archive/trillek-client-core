#include "trillek-game.hpp"

namespace trillek {

TrillekScheduler TrillekGame::scheduler;
FakeSystem TrillekGame::fake_system;
OS TrillekGame::glfw_os;
std::once_flag TrillekGame::once_graphics;
std::shared_ptr<graphics::RenderSystem> TrillekGame::gl_sys_ptr;
bool TrillekGame::close_window = false;

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
