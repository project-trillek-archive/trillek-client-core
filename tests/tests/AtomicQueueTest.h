#ifndef ATOMICQUEUETEST_H_INCLUDED
#define ATOMICQUEUETEST_H_INCLUDED

#include "engine/core/AtomicQueue.hpp"

#include "gtest/gtest.h"

class AtomicQueueTest: public ::testing::Test {
public:
    AtomicQueueTest() {};
protected:
    trillek::AtomicQueue<int> q;
};

using trillek::AtomicQueue;

namespace trillek {
    TEST_F(AtomicQueueTest, AtomicQueueEmpty) {
        ASSERT_TRUE(q.Empty()) << "New queue is not empty";
        int i = 0;
        ASSERT_FALSE(q.Pop(i)) << "New queue can pop inexisting element";
        ASSERT_EQ(i, 0) << "New queue popped  an element";
        ASSERT_TRUE(q.Poll().empty()) << "New polled queue gives elements";
    }

    TEST_F(AtomicQueueTest, AtomicQueueOneElement) {
        q.Push(1);
        ASSERT_FALSE(q.Empty()) << "Queue is empty";
        int i = 0;
        ASSERT_TRUE(q.Pop(i)) << "Queue can't pop existing element";
        ASSERT_EQ(i, 1) << "Queue popped  wrong value";
        ASSERT_TRUE(q.Empty()) << "Queue is not empty";
        ASSERT_TRUE(q.Poll().empty()) << "Polled queue gives elements";
    }

    TEST_F(AtomicQueueTest, AtomicQueuePoll) {
        q.Push(1);
        q.Push(2);
        ASSERT_FALSE(q.Empty()) << "Queue is empty";
        auto ret = q.Poll();
        ASSERT_TRUE(q.Empty()) << "Queue is not empty";
        ASSERT_FALSE(ret.empty()) << "Returned list from Poll() is empty";
        ASSERT_EQ(ret.front(), 1) << "First element from Poll has wrong value";
        ret.pop_front();
        ASSERT_FALSE(ret.empty()) << "Returned list from Poll() has only one element";
        ASSERT_EQ(ret.front(), 2) << "Second element from Poll has wrong value";
        ret.pop_front();
        ASSERT_TRUE(ret.empty()) << "Returned list from Poll() has more than 2 elements";
    }

    TEST_F(AtomicQueueTest, AtomicQueuePop) {
        q.Push(1);
        q.Push(2);
        ASSERT_FALSE(q.Empty()) << "Queue is empty";
        int i = 0;
        ASSERT_TRUE(q.Pop(i)) << "Queue can't pop existing element";
        ASSERT_EQ(i, 1) << "Pop()  wrong value";
        ASSERT_FALSE(q.Empty()) << "Queue is empty";
        ASSERT_TRUE(q.Pop(i)) << "Queue can't pop existing element";
        ASSERT_EQ(i, 2) << "Pop()  wrong value";
        ASSERT_TRUE(q.Empty()) << "Queue is not empty";
        ASSERT_TRUE(q.Poll().empty()) << "Empty queue gives elements";
    }

    TEST_F(AtomicQueueTest, AtomicQueueList) {
        std::list<int> a{1,2,3,4,5};
        q.PushList(a);
        ASSERT_FALSE(q.Empty()) << "Queue is empty";
        auto ret = q.Poll();
        ASSERT_TRUE(q.Empty()) << "Queue is not empty";
        ASSERT_EQ(ret.size(), 5) << "Poll does not return all elements";
        for (auto i = 1; i < 6; ++i) {
            ASSERT_FALSE(ret.empty()) << "Returned list from Poll() is empty";
            ASSERT_EQ(ret.front(), i) << i << "th element from Poll has wrong value";
            ret.pop_front();
        }
        ASSERT_TRUE(q.Empty()) << "Queue is not empty";
        ASSERT_TRUE(q.Poll().empty()) << "Polled queue gives elements";
    }
}
#endif // ATOMICQUEUETEST_H_INCLUDED
