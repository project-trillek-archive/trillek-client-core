#include "trillek-game.hpp"

namespace trillek {
TrillekScheduler TrillekGame::scheduler;
FakeSystem TrillekGame::fake_system;
OS TrillekGame::glfw_os;
graphics::RenderSystem TrillekGame::gl_sys;
physics::PhysicsSystem TrillekGame::phys_sys;
MetaEngineSystem TrillekGame::engine_sys;
bool TrillekGame::close_window = false;
}
