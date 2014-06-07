#include "trillek-game.hpp"

namespace trillek {
TrillekScheduler TrillekGame::scheduler;
FakeSystem TrillekGame::fake_system;
OS TrillekGame::glfw_os;
graphics::System TrillekGame::gl_sys;
bool TrillekGame::close_window = false;
}