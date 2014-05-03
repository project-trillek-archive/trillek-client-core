#include "TrillekGame.h"

namespace trillek {
    TrillekScheduler TrillekGame::scheduler;
    FakeSystem TrillekGame::fake_system;
    OS TrillekGame::glfw_os;
    bool TrillekGame::terminateFlag = false;
}