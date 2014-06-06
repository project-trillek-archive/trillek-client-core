#include "trillek-game.hpp"
#include <queue>
#include <thread>
#include <chrono>
#include "os.hpp"
#include "systems/json-parser.hpp"
#include "systems/transform-system.hpp"
#include "systems/resource-system.hpp"
#include "systems/graphics.hpp"
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
    trillek::transform::System::GetInstance();
    trillek::resource::System::GetInstance();

    trillek::json::System jparser;
    jparser.Parse("assets/tests/sample.json");

    // start the graphic system
    trillek::TrillekGame::GetGraphicSystem().Start(os.GetWindowWidth(), os.GetWindowHeight());

    // we register the systems in this queue
    std::queue<trillek::SystemBase*> systems;

    // register the fake system. Comment this to cancel
    systems.push(&trillek::TrillekGame::GetFakeSystem());

    // register the graphic system
    systems.push(&trillek::TrillekGame::GetGraphicSystem());

    // Detach the window from the current thread
    os.DetachContext();

    // start the scheduler in another thread
    std::thread tp(
                   &trillek::TrillekScheduler::Initialize,
                   &trillek::TrillekGame::GetScheduler(),
                   5,
                   std::ref(systems));
    while (! os.Closing()) {
        os.OSMessageLoop();
    }
    tp.join();

    // Terminating program
    os.MakeCurrent();
    os.Terminate();
    return 0;
}
