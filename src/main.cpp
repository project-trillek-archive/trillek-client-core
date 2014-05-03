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
    // TODO: Tell the user we are waiting all systems to terminate
    std::this_thread::sleep_for(std::chrono::seconds(1));
    os.Terminate();
    return 0;
}
