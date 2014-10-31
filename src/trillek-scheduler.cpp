#include "trillek-scheduler.hpp"
#include <thread>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <functional>
#include <algorithm>

#include "systems/system-base.hpp"
#include "trillek-game.hpp"
#include "os.hpp"
#include "logging.hpp"

namespace trillek {
std::function<void(std::shared_ptr<TaskRequest<chain_t>>&&,frame_unit&&)> TaskRequest<chain_t>::queue_task;

scheduler_tp TaskRequestBase::Now() const {
#if defined(_MSC_VER)
    return scheduler_tp(TrillekGame::GetOS().GetTime());
#else
//    return scheduler_tp(TrillekGame::GetOS().GetTime());
    return scheduler_tp{std::chrono::steady_clock::now()};
#endif
}


void TrillekScheduler::Initialize(unsigned int nr_thread, std::queue<SystemBase*>& systems) {
    std::list<std::thread> thread_list;
    // initialize
#if defined(_MSC_VER)
    scheduler_tp now = scheduler_tp(TrillekGame::GetOS().GetTime());
#else
    scheduler_tp now{std::chrono::steady_clock::now()};
#endif
    TaskRequest<chain_t>::Initialize([&](std::shared_ptr<TaskRequest<chain_t>>&& c, frame_unit&& delay)
                                    {
                                        c->Reschedule(std::move(delay));
                                        Queue(std::move(c));
                                    });
    // prepare threads
    for (unsigned int i = 0; i < nr_thread; ++i) {
        SystemBase* sys = nullptr;
        if (! systems.empty()) {
            sys = systems.front();
            systems.pop();
        }
        auto f = std::bind(&TrillekScheduler::DayWork, std::ref(*this), now, sys);
        thread_list.push_back(std::thread(std::move(f)));
    }
    // run threads and block
    for (auto& t : thread_list) {
        t.join();
    }
}

void TrillekScheduler::DayWork(const scheduler_tp& now, SystemBase* system) {
    scheduler_tp next_frame_tp = now + one_frame;

    std::function<void(frame_tp)> handleEvents_functor;
    std::function<void(void)> runBatch_functor;
    std::function<void(void)> terminate_functor;
    if (system) {
        handleEvents_functor = std::bind(&SystemBase::HandleEvents, std::ref(*system), std::placeholders::_1);
        runBatch_functor = std::bind(&SystemBase::RunBatch, std::cref(*system));
        terminate_functor = std::bind(&SystemBase::Terminate, std::ref(*system));
        system->ThreadInit();
    } else {
        handleEvents_functor = [](frame_tp timepoint) {};
        runBatch_functor = [] () {};
        terminate_functor = [] () {};
    }

    while (1) {
        std::shared_ptr<TaskRequestBase> task;

        {
            // Wait for a task to do
            // a lock to watch the blocking point
            std::unique_lock<std::mutex> locker(m_timer);
            // lock the queue while checking it
            m_queue.lock();
#if defined(_MSC_VER)
            while (taskqueue.empty()
                   || ! taskqueue.top()->IsNow()
                   || scheduler_tp(TrillekGame::GetOS().GetTime()) >= next_frame_tp) {
#else
            while (taskqueue.empty()
                   || ! taskqueue.top()->IsNow()
                   || std::chrono::steady_clock::now() >= next_frame_tp) {
#endif
                const auto max_timepoint = taskqueue.empty() ? next_frame_tp :
                                                            (std::min)(next_frame_tp, taskqueue.top()->Timepoint());
                // unlock the queue when checking is done
                m_queue.unlock();
                // threads wait here (blocking point)
                if (queuecheck.wait_until(locker, max_timepoint) == std::cv_status::timeout) {
                    // we are here because we reached a timeout
                    // release the lock of the blocking point
                    m_timer.unlock();
                    if (TrillekGame::GetTerminateFlag()) {
                        LOGMSGC(INFO) << "Scheduler: Terminate signal detected for this thread...";
                        // unblock all other threads waiting above
                        queuecheck.notify_all();
                        // save the state of the system
                        terminate_functor();
                        // lock the mutex before exiting the block
                        m_timer.lock();
                        return;
                    }
#if defined(_MSC_VER)
                    if (scheduler_tp(TrillekGame::GetOS().GetTime()) >= next_frame_tp) {
#else
                    if (std::chrono::steady_clock::now() >= next_frame_tp) {
#endif
                        // a new frame has begun : let's run the system
                        handleEvents_functor(next_frame_tp.time_since_epoch().count());
                        runBatch_functor();
                        next_frame_tp += one_frame;
                    }
                    // reacquire the lock of the blocking point
                    m_timer.lock();
                }
                // we arrive directly here if a new task has been queued
                // relock the queue before checking it
                m_queue.lock();
            }
        }
        // Get the task to do
        task = taskqueue.top();
        taskqueue.pop();
        m_queue.unlock();

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
        countercheck.notify_all();
    }
}
}
