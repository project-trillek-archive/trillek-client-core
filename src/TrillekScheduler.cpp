#include "TrillekScheduler.h"
#include <thread>
//#include <iostream>
//#include <utmpx.h>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <functional>

#include "System.h"

namespace trillek {
    std::function<void(std::shared_ptr<TaskRequest<chain_t>>&&,frame_unit&&)> TaskRequest<chain_t>::queue_task;

    void TrillekScheduler::Initialize(unsigned int nr_thread, std::queue<System*>&& systems) {
//        LOG_DEBUG << "hardware concurrency " << std::thread::hardware_concurrency();
        frame_tp now = steady_clock::now();
        for (auto i = 0; i < nr_thread; ++i) {
            System* sys = nullptr;
            if (! systems.empty()) {
                sys = systems.front();
                systems.pop();
            }
            std::thread(std::bind(&TrillekScheduler::DayWork, std::ref(*this), now, sys)).detach();
        }
        TaskRequest<chain_t>::Initialize([&](std::shared_ptr<TaskRequest<chain_t>>&& c, frame_unit&& delay)
                                        {
                                            c->Reschedule(std::move(delay));
                                            Queue(std::move(c));
                                        });
    }

    void TrillekScheduler::DayWork(const frame_tp& now, System* system) {
        frame_tp next_frame_tp = now + one_frame;

        std::function<void(const frame_tp&)> handleEvents_functor;
        std::function<void(void)> runBatch_functor;
        if (system) {
            handleEvents_functor = std::bind(&System::HandleEvents, std::ref(*system), std::placeholders::_1);
            runBatch_functor = std::bind(&System::RunBatch, std::cref(*system));
        } else {
            handleEvents_functor = [](const frame_tp& timepoint) {};
            runBatch_functor = [] () {};
        }

        while (1) {

            std::shared_ptr<TaskRequestBase> task;

            {
                // Wait for a task to do
                std::unique_lock<std::mutex> locker(m_queue);
                while (taskqueue.empty()
                       || ! taskqueue.top()->IsNow()
                       || steady_clock::now() >= next_frame_tp) {
                    auto max_timepoint = taskqueue.empty() ? next_frame_tp :
                                                                std::min(next_frame_tp, taskqueue.top()->Timepoint());
                    if (queuecheck.wait_until(locker, max_timepoint) == std::cv_status::timeout) {
                        // we reach timeout
                        if (steady_clock::now() >= next_frame_tp) {
                            handleEvents_functor(next_frame_tp);
                            runBatch_functor();
                            next_frame_tp += one_frame;
                        }
                    }
                }
                // Get the task to do
                task = taskqueue.top();
                taskqueue.pop();
            }
            {
                std::unique_lock<std::mutex> locker(m_count);
                while(counter >= MAX_CONCURRENT_THREAD) {
                    // Wait the counter to be under MAX_CONCURRENT_THREAD
                    countercheck.wait(locker, [&](){return counter < MAX_CONCURRENT_THREAD;});
                }
                // increment counter when possible
                counter.fetch_add(1, std::memory_order::memory_order_relaxed);
            }

            task->RunTask();

            // decrement counter
            counter.fetch_sub(1, std::memory_order::memory_order_relaxed);
            queuecheck.notify_all();
        }
    }
}
