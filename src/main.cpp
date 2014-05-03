#include "TrillekGame.h"
#include <queue>
#include <thread>
#include <chrono>

int main(int argCount, char **argValues) {
    std::queue<trillek::System*> systems;
    systems.push(&trillek::TrillekGame::GetFakeSystem());
    trillek::TrillekGame::GetScheduler().Initialize(5, std::move(systems));

    auto& os = trillek::TrillekGame::GetOS();

    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 0);

    while (!os.Closing()) {
        os.OSMessageLoop();
    }

    os.Terminate();
    return 0;
}
