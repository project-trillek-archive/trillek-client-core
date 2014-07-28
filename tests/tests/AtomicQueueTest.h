#ifndef ATOMICQUEUETEST_H_INCLUDED
#define ATOMICQUEUETEST_H_INCLUDED

#include "atomic-queue.hpp"

#include "gtest/gtest.h"

class AtomicQueueTest: public ::testing::Test {
public:
    AtomicQueueTest() {};
protected:
    trillek::AtomicQueue<uint32_t> q;
};

using trillek::AtomicQueue;

namespace trillek {
TEST_F(AtomicQueueTest, AtomicQueueEmpty) {
    ASSERT_TRUE(q.Empty()) << "New queue is not empty";
    uint32_t i = 0;
    ASSERT_FALSE(q.Pop(i)) << "New queue can pop inexisting element";
    ASSERT_EQ(i, 0) << "New queue popped  an element";
    ASSERT_TRUE(q.Poll().empty()) << "New polled queue gives elements";
}

TEST_F(AtomicQueueTest, AtomicQueueOneElement) {
    q.Push(1);
    ASSERT_FALSE(q.Empty()) << "Queue is empty";
    uint32_t i = 0;
    ASSERT_TRUE(q.Pop(i)) << "Queue can't pop existing element";
    ASSERT_EQ(i, 1) << "Queue popped  wrong value";
    ASSERT_TRUE(q.Empty()) << "Queue is not empty";
    ASSERT_TRUE(q.Poll().empty()) << "Polled queue gives elements";
    ASSERT_EQ(gAllocatedSize, 0) << "Allocated size is not null";
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
    ASSERT_EQ(gAllocatedSize, 0) << "Allocated size is not null";
}

TEST_F(AtomicQueueTest, AtomicQueuePop) {
    q.Push(1);
    q.Push(2);
    ASSERT_FALSE(q.Empty()) << "Queue is empty";
    uint32_t i = 0;
    ASSERT_TRUE(q.Pop(i)) << "Queue can't pop existing element";
    ASSERT_EQ(i, 1) << "Pop()  wrong value";
    ASSERT_FALSE(q.Empty()) << "Queue is empty";
    ASSERT_TRUE(q.Pop(i)) << "Queue can't pop existing element";
    ASSERT_EQ(i, 2) << "Pop()  wrong value";
    ASSERT_TRUE(q.Empty()) << "Queue is not empty";
    ASSERT_TRUE(q.Poll().empty()) << "Empty queue gives elements";
    ASSERT_EQ(gAllocatedSize, 0) << "Allocated size is not null";
}

TEST_F(AtomicQueueTest, AtomicQueueCopyList) {
    std::list<uint32_t, TrillekAllocator<uint32_t>> a{1,2,3,4,5};
    q.PushList(a);
    ASSERT_FALSE(q.Empty()) << "Target queue is empty";
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

TEST_F(AtomicQueueTest, AtomicQueueMoveList) {
    auto alloc_backup = gAllocatedSize;
    std::list<uint32_t, TrillekAllocator<uint32_t>> a{1,2,3,4,5};
    q.PushList(std::move(a));
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
