#include "TrillekGame.h"
#include <queue>
#include <thread>
#include <chrono>

int main(int argCount, char **argValues) {
    auto& os = trillek::TrillekGame::GetOS();
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 0);

    std::queue<trillek::System*> systems;
    systems.push(&trillek::TrillekGame::GetFakeSystem());
    trillek::TrillekGame::GetScheduler().Initialize(5, std::move(systems));

    while (!os.Closing()) {
        os.OSMessageLoop();
    }
    trillek::TrillekGame::SetTerminateFlag();
    // TODO: Tell the user we are waiting all systems to terminate
    std::this_thread::sleep_for(std::chrono::seconds(1));
    os.Terminate();
    return 0;
}
