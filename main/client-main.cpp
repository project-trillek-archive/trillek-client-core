#include "trillek-game.hpp"
#include <queue>
#include <thread>
#include "os.hpp"
#include "util/json-parser.hpp"
#include "systems/transform-system.hpp"
#include "systems/resource-system.hpp"
#include "systems/physics.hpp"
#include "systems/meta-engine-system.hpp"
#include "systems/sound-system.hpp"
#include "systems/graphics.hpp"
#include <iostream>

#include "systems/vcomputer-system.hpp"

size_t gAllocatedSize = 0;

int main(int argCount, char **argValues) {
    trillek::TrillekGame::Initialize();
    std::cout << "Starting Trillek client..." << std::endl;
    // create the window
    auto& os = trillek::TrillekGame::GetOS();
#if __APPLE__
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 2);
#else
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 0);
#endif
    glGetError(); // clear errors

    // Call each system's GetInstance to create the initial instance.
    trillek::TransformMap::GetInstance();
    trillek::resource::ResourceMap::GetInstance();

    // start the physics system, must be done before loading any components.
    trillek::TrillekGame::GetPhysicsSystem().Start();

    trillek::util::JSONPasrser jparser;

    if (!jparser.Parse("common/assets/tests/sample.json")) {
        std::cerr << "Error loading JSON configuration file." << std::endl;
    }

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
//  systems.push(&trillek::TrillekGame::GetFakeSystem());

    // register the engine system, i.e graphics + physics
    systems.push(&trillek::TrillekGame::GetEngineSystem());

    // register the sound system
    systems.push(&trillek::TrillekGame::GetSoundSystem());

    // Start Lua system.
    trillek::TrillekGame::GetLuaSystem().Start();

    // Load a test file/main Lua file.
    trillek::TrillekGame::GetLuaSystem().LoadFile("common/assets/scripts/test.lua");

    // Detach the window from the current thread
    os.DetachContext();

    trillek::VComputerSystem cpu1;
    systems.push(&cpu1);

    // start the scheduler in another thread
    std::thread tp(
                   &trillek::TrillekScheduler::Initialize,
                   &trillek::TrillekGame::GetScheduler(),
                   5,
                   std::ref(systems));

    // Start the client network layer
/*    trillek::TrillekGame::GetNetworkClient().SetTCPHandler<trillek::network::CLIENT>();

    // Start the server network layer and connect the client to the server
    if(! trillek::TrillekGame::GetNetworkClient().Connect("localhost", 7777, "my_login", "secret password")) {
        trillek::TrillekGame::NotifyCloseWindow();
    }
*/
    while (! os.Closing()) {
        os.OSMessageLoop();
    }
    tp.join();

    // Terminating program
    os.MakeCurrent();
    os.Terminate();

    // TODO: fix rotation.y for entity 1001 becoming NaN
    //jparser.Serialize("common/assets/tests/", "transforms.json", trillek::TransformMap::GetInstance());
    std::cout << "Number of bytes not freed: " << gAllocatedSize << std::endl;
    return 0;
}
