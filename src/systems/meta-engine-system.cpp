#include "systems/meta-engine-system.hpp"
#include "trillek-game.hpp"

namespace trillek {
void MetaEngineSystem::ThreadInit() {
    TrillekGame::GetPhysicsSystem().ThreadInit();
    TrillekGame::GetGraphicSystem().ThreadInit();
}

void MetaEngineSystem::RunBatch() const {
    TrillekGame::GetPhysicsSystem().RunBatch();
    TrillekGame::GetGraphicSystem().RunBatch();
};

void MetaEngineSystem::HandleEvents(const frame_tp& timepoint) {
    TrillekGame::GetPhysicsSystem().HandleEvents(timepoint);
    TrillekGame::GetGraphicSystem().HandleEvents(timepoint);
};

void MetaEngineSystem::Terminate() {
    TrillekGame::GetPhysicsSystem().Terminate();
    TrillekGame::GetGraphicSystem().Terminate();
};
}