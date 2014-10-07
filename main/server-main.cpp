#include "trillek-game.hpp"
#include <queue>
#include <thread>

#if defined(_MSC_VER)
// Visual Studio implements steady_clock as system_clock
// We need GLFW to geta decent clock
// TODO : wait for the fix from Microsoft
#include "os.hpp"
#endif

#include "util/json-parser.hpp"
#include "systems/transform-system.hpp"
#include "systems/resource-system.hpp"
#include "systems/physics.hpp"
#include <cstddef>
#include <iostream>

size_t gAllocatedSize = 0;

int main(int argCount, char **argValues) {
    trillek::TrillekGame::Initialize();
#ifdef _SERVER_
    std::cout << "Starting Trillek server..." << std::endl;
#endif // _SERVER_

#if defined(_MSC_VER)
    // Visual C++ rely on GLFW clock
    // create the window
    auto& os = trillek::TrillekGame::GetOS();
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 0);
    glGetError(); // clear errors
#endif

    // Call each system's GetInstance to create the initial instance.
    trillek::TransformMap::GetInstance();
    trillek::resource::ResourceMap::GetInstance();

    // start the physics system, must be done before loading any components.
    trillek::TrillekGame::GetPhysicsSystem().Start();

    trillek::util::JSONPasrser jparser;

    if (!jparser.Parse("assets/tests/sample.json")) {
        std::cerr << "Error loading JSON configuration file." << std::endl;
    }

    // we register the systems in this queue
    std::queue<trillek::SystemBase*> systems;

    // register the fake system. Comment this to cancel
//  systems.push(&trillek::TrillekGame::GetFakeSystem());

    // register the physics system
    systems.push(&trillek::TrillekGame::GetPhysicsSystem());

    // start the scheduler in another thread
    std::thread tp(
                   &trillek::TrillekScheduler::Initialize,
                   &trillek::TrillekGame::GetScheduler(),
                   5,
                   std::ref(systems));
    // Start the server network layer and connect the client to the server
/*    trillek::network::NetworkController &server = trillek::TrillekGame::GetNetworkServer();
    server.SetTCPHandler<trillek::network::SERVER>();
    server.Server_Start("localhost", 7777);
    std::this_thread::sleep_for(std::chrono::seconds(1));
*/
#if defined(_MSC_VER)
    while (! os.Closing()) {
        os.OSMessageLoop();
    }
    tp.join();
    os.Terminate();
#else
    tp.join();
#endif

    return 0;
}
