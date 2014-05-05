#ifndef TRILLEKSCHEDULER_H_INCLUDED
#define TRILLEKSCHEDULER_H_INCLUDED

#define MAX_CONCURRENT_THREAD   4

#include <chrono>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <queue>
#include <list>
#include <iterator>
#include "AtomicQueue.hpp"

#define		STOP		0
#define 	SPLIT		1
#define		CONTINUE	2
#define		REQUEUE		3
#define		REPEAT		4

namespace trillek {

    using namespace std::chrono;

    class System;

    typedef std::function<int(void)> block_t;
    typedef std::list<block_t> chain_t;

    typedef duration<double, std::ratio<1,60>> frame_unit;
    typedef time_point<system_clock, frame_unit> frame_tp;
    typedef time_point<system_clock, duration<double, std::ratio<1>>> glfw_tp;

    class TaskRequestBase {
    public:
        TaskRequestBase(frame_tp&& timestamp) :
            timestamp(std::move(timestamp))
            {};

        virtual ~TaskRequestBase() {};

        bool operator<(const TaskRequestBase& tqe) const {
            return this->timestamp < tqe.timestamp;
        }

        virtual void RunTask() = 0;

        glfw_tp Now() const;

        bool IsNow() const {
            return timestamp < Now();
        }

        void Reschedule(frame_unit&& delay) {
            timestamp = Now() + delay;
        }

        frame_tp Timepoint() const {
            return timestamp;
        }

    protected:
        frame_tp timestamp;
    };

    template<class T>
    class TaskRequest : public TaskRequestBase {
    public:
        TaskRequest(T&& funct) :
            funct(std::forward<T>(funct)),
            TaskRequestBase(Now())
            {};

        TaskRequest(T&& funct, const frame_unit& delay) :
            funct(std::forward<T>(funct)),
            TaskRequestBase(Now() + delay)
            {};

        virtual ~TaskRequest() {};

        void RunTask() override {
            funct();
        }
    private:
        const T funct;
    };

    template<>
    class TaskRequest<chain_t> : public TaskRequestBase {
    public:
        TaskRequest(const chain_t& chain) :
            block(chain.cbegin()),
            block_end(chain.cend()),
            TaskRequestBase(Now())
            {};

        TaskRequest(const chain_t& chain, const frame_unit& delay) :
            block(chain.cbegin()),
            block_end(chain.cend()),
            TaskRequestBase(Now() + delay)
            {};

        TaskRequest(chain_t&& chain) = delete;

        TaskRequest(chain_t&& chain, const frame_unit& delay) = delete;

        TaskRequest(std::shared_ptr<chain_t>&& chain) :
            chain(std::move(chain)),
            block(this->chain->cbegin()),
            block_end(this->chain->cend()),
            TaskRequestBase(Now())
            {};

        TaskRequest(std::shared_ptr<chain_t>&& chain, const frame_unit& delay) :
            chain(std::move(chain)),
            block(this->chain->cbegin()),
            block_end(this->chain->cend()),
            TaskRequestBase(Now() + delay)
            {};

        virtual ~TaskRequest() {};

        TaskRequest<chain_t>& operator++() {
            if (block_end != block) {
                ++block;
            }
        }

        void RunTask() override {
            for(auto& b = block; block_end != b; ++b) {
                auto s = (*b)();
                switch(s) {
                case REQUEUE:
                    // "*this" is now undefined
                    queue_task(std::make_shared<TaskRequest<chain_t>>(std::move(*this)), frame_unit(0.1));
                case STOP:
                    return;
                case SPLIT:
                    // Queue a thread to execute this block again, and continue the chain
                    queue_task(std::make_shared<TaskRequest<chain_t>>(*this), frame_unit(0.1));
                    break;
                case REPEAT:
                    --b;
                case CONTINUE:
                default:
                    break;
                }
            }
        }

        static void Initialize(std::function<void(std::shared_ptr<TaskRequest<chain_t>>&&, frame_unit&&)>&& f) {
            queue_task = std::move(f);
        };

    private:
        static std::function<void(std::shared_ptr<TaskRequest<chain_t>>&&, frame_unit&&)> queue_task;
        const std::shared_ptr<chain_t> chain;
        chain_t::const_iterator block;
        const chain_t::const_iterator block_end;
    };


    /** \brief Scheduler for trillek engine
     */
    class TrillekScheduler {
    public:
        TrillekScheduler() : counter(0), one_frame(1) {};
        virtual ~TrillekScheduler() {};

        /** \brief Launch the threads and attach them to system
         *
         * \param nr_thread unsigned int number of threads to launch
         * \param systems std::queue<System*>&& list of systems to attach
         * \param m std::mutex& a mutex held while there is still a running thread
         *
         */
        void Initialize(unsigned int nr_thread, std::queue<System*>&& systems, std::mutex& m);

        /** \brief Execute a task using the current thread
         *
         * \param task task to execute
         *
         */
        template<class T>
        void Execute(const std::shared_ptr<TaskRequest<T>>& task) {
            task->RunTask();
        }

        /** \brief Queue a task for asynchronous execution
         *
         * \param task task to execute
         *
         */
        template<class T>
        void Queue(T&& task) {
            std::unique_lock<std::mutex> locker(m_queue);
            taskqueue.push(std::forward<T>(task));
            queuecheck.notify_one();
        }

    private:

        /** \brief Main loop of each thread
         *
         * \param now start time
         * \param system System* system to attach
         *
         */
        void DayWork(const frame_tp& now, System* system);

        std::priority_queue<std::shared_ptr<TaskRequestBase>> taskqueue;
        std::condition_variable countercheck;
        std::atomic<int> counter;
        std::mutex m_count;
        std::mutex m_queue;
        std::condition_variable queuecheck;
        const frame_unit one_frame;
    };
}

#endif // TRILLEKSCHEDULER_H_INCLUDED
