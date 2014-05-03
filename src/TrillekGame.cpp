#include "TrillekGame.h"

namespace trillek {
    TrillekScheduler TrillekGame::scheduler;
    FakeSystem TrillekGame::fake_system;
    OS TrillekGame::glfw_os;
    std::condition_variable TrillekGame::close_window;
}