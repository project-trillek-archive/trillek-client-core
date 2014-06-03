#include "TrillekGame.h"
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

    // mutex held by the scheduler
    std::mutex scheduler_terminating_mutex;
    // start the scheduler in another thread
    std::thread tp(
                   &trillek::TrillekScheduler::Initialize,
                   &trillek::TrillekGame::GetScheduler(),
                   5,
                   std::move(systems),
                   std::ref(scheduler_terminating_mutex));
    tp.detach();

    // schedule the OS loop task
    auto olf = new trillek::chain_t{std::bind(&trillek::OS::OSMessageLoop, std::ref(os))};
    auto os_loop_functor = std::make_shared<trillek::TaskRequest<trillek::chain_t>>(std::shared_ptr<trillek::chain_t>(olf));
    trillek::TrillekGame::GetScheduler().Queue(std::move(os_loop_functor));

    // wait the user to close the window
    std::mutex closing_window_mutex;
    std::unique_lock<std::mutex> locker(closing_window_mutex);
    while (!os.Closing()) {
        // the thread is blocked here
        trillek::TrillekGame::GetCloseWindowCV().wait(locker, [&]() { return os.Closing(); });
    }

    // wait the scheduler thread to terminate
    while (! scheduler_terminating_mutex.try_lock()) {
        // TODO: Display something on the screen
    };

    trillek::TrillekGame::GetGraphicSystem().Terminate();
    os.Terminate();
    return 0;
}
