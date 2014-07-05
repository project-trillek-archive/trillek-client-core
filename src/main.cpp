#include "trillek-game.hpp"
#include <queue>
#include <thread>
#include <chrono>
#include "os.hpp"
#include "util/json-parser.hpp"
#include "systems/transform-system.hpp"
#include "systems/resource-system.hpp"
#include "systems/meta-engine-system.hpp"
#include "systems/sound-system.hpp"
#include <cstddef>

size_t gAllocatedSize = 0;

int main(int argCount, char **argValues) {
    // create the window
    auto& os = trillek::TrillekGame::GetOS();
#if __APPLE__
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 2);
#else
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 0);
#endif

    // Call each system's GetInstance to create the initial instance.
    trillek::TransformMap::GetInstance();
    trillek::resource::ResourceMap::GetInstance();

    // start the physics system, must be done before loading any components.
    trillek::TrillekGame::GetPhysicsSystem().Start();

    trillek::util::JSONPasrser jparser;
    jparser.Parse("assets/tests/sample.json");

    trillek::sound::System& soundsystem = trillek::TrillekGame::GetSoundSystem();
    std::shared_ptr<trillek::sound::Sound> s1 = soundsystem.GetSound("music_track_1");
    // needs to be a mono sound for 3d effects to work
    if (s1) {
        s1->Play();
    }
    // start the graphic system
    trillek::TrillekGame::GetGraphicSystem().Start(os.GetWindowWidth(), os.GetWindowHeight());

    // we register the systems in this queue
    std::queue<trillek::SystemBase*> systems;

    // register the fake system. Comment this to cancel
//    systems.push(&trillek::TrillekGame::GetFakeSystem());

    // register the engine system, i.e graphics + physics
    systems.push(&trillek::TrillekGame::GetEngineSystem());

    // register the sound system
    systems.push(&trillek::TrillekGame::GetSoundSystem());

    // Detach the window from the current thread
    os.DetachContext();

    // start the scheduler in another thread
    std::thread tp(
                   &trillek::TrillekScheduler::Initialize,
                   &trillek::TrillekGame::GetScheduler(),
                   5,
                   std::ref(systems));
    while (!os.Closing()) {
        os.OSMessageLoop();
    }
    tp.join();

    // Terminating program
    os.MakeCurrent();
    os.Terminate();
    return 0;
}
