#if defined(_CLIENT_) || defined(_STANDALONE_)
#include "trillek-game.hpp"
#include "systems/meta-engine-system.hpp"
#include "systems/physics.hpp"
#include "systems/graphics.hpp"

namespace trillek {
void MetaEngineSystem::ThreadInit() {
    TrillekGame::GetPhysicsSystem().ThreadInit();
    TrillekGame::GetGraphicSystem().ThreadInit();
}

void MetaEngineSystem::RunBatch() const {
    TrillekGame::GetPhysicsSystem().RunBatch();
    TrillekGame::GetGraphicSystem().RunBatch();
};

void MetaEngineSystem::HandleEvents(frame_tp timepoint) {
    TrillekGame::GetPhysicsSystem().HandleEvents(timepoint);
    TrillekGame::GetGraphicSystem().HandleEvents(timepoint);
};

void MetaEngineSystem::Terminate() {
    TrillekGame::GetPhysicsSystem().Terminate();
    TrillekGame::GetGraphicSystem().Terminate();
};
}
#endif
