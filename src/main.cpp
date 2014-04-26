#include "engine/core/TrillekGame.h"
#include <queue>
#include <thread>
#include <chrono>

int main(int argCount, char **argValues) {
    std::queue<trillek::System*> systems;
    systems.push(&trillek::TrillekGame::GetFakeSystem());
    trillek::TrillekGame::GetScheduler().Initialize(5, std::move(systems));
    std::this_thread::sleep_for(std::chrono::seconds(20));
    return 0;
}
