#include "TrillekGame.h"

namespace trillek {
    TrillekScheduler TrillekGame::scheduler;
    FakeSystem TrillekGame::fake_system;
    OS TrillekGame::glfw_os;
    graphics::System TrillekGame::gl_sys;
    std::condition_variable TrillekGame::close_window;
}