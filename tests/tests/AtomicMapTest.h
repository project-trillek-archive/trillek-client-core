#ifndef ATOMICMAPTEST_H_INCLUDED
#define ATOMICMAPTEST_H_INCLUDED

#include "AtomicMap.hpp"
#include <stdexcept>

#include "gtest/gtest.h"

    class AtomicMapTest: public ::testing::Test {
	public:
		AtomicMapTest() {};
    protected:
        trillek::AtomicMap<std::string, int> q;
    };

using trillek::AtomicMap;

namespace trillek {
    TEST_F(AtomicMapTest, AtomicMapEmpty) {
        ASSERT_EQ(q.Count("a"), 0) << "New map is not empty";
        int i = 0;
        ASSERT_FALSE(q.Pop("a", i)) << "New map can pop inexisting element";
        ASSERT_EQ(i, 0) << "New map popped  an element";
        ASSERT_FALSE(q.Compare("a", 1)) << "Comparison in empty map should return false";
        EXPECT_THROW(i = q.At("a"), std::out_of_range);
    }

    TEST_F(AtomicMapTest, AtomicMapOneElement) {
        auto k = "a";
        int i = 1;
        q.Insert(k, i);
        ASSERT_EQ(k, "a") << "Insert moved lvalue key";
        ASSERT_EQ(i, 1) << "Insert moved lvalue value";
        ASSERT_FALSE(q.Count(k) == 0) << "Map is empty";
        EXPECT_NO_THROW(i = q.At("a")) << "At throws an exception";
        ASSERT_TRUE(q.Compare("a", 1)) << "Compare should return true";
        ASSERT_FALSE(q.Compare("a", 2)) << "Compare should return false";
        i = 0;
        ASSERT_TRUE(q.Pop("a", i)) << "Map can't pop existing element";
        ASSERT_EQ(i, 1) << "Map popped  wrong value";
        ASSERT_EQ(q.Count(k), 0) << "Map is not empty";
        EXPECT_THROW(i = q.At("a"), std::out_of_range) << "At does not throw when it should";
    }

    TEST_F(AtomicMapTest, AtomicMapErase) {
        q.Insert("a", 1);
        q.Erase("a");
        ASSERT_EQ(q.Count("a"), 0) << "Map is not empty";
    }
}
#endif // ATOMICMAPTEST_H_INCLUDED
