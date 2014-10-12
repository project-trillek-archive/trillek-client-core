#ifndef REWINDABLE_MAP_TEST_HPP_INCLUDED
#define REWINDABLE_MAP_TEST_HPP_INCLUDED

#include "systems/rewindable-map.hpp"
#include "gtest/gtest.h"

class RewindableMapTest : public ::testing::Test {
public:
    void SetUp() override {
        refmap0.emplace(1,std::string("one"));
        refmap0.emplace(2,std::string("two"));
        refmap0.emplace(3,std::string("three"));
        refmap0.emplace(4,std::string("four"));
        refmap0.emplace(5,std::string("five"));
        refmap100.emplace(1,std::string("two"));
        refmap100.emplace(2,std::string("one"));
        refmap100.emplace(3,std::string("three"));
        refmap100.emplace(5,std::string("three"));
        refmap200.emplace(1,std::string("two"));
        refmap200.emplace(2,std::string("three"));
        refmap200.emplace(3,std::string("two"));
        refmap200.emplace(4,std::string("six"));
        refmap200.emplace(5,std::string("one"));
        rmap.Insert(1,std::string("one"));
        rmap.Insert(2,std::string("two"));
        rmap.Insert(3,std::string("three"));
        rmap.Insert(4,std::string("four"));
        rmap.Insert(5,std::string("five"));
    }

    void Modify(trillek::RewindableMap<unsigned int,std::string,int64_t,50>& data) {
        data.Update(2,std::string("one"));
        data.Update(1,std::string("two"));
        data.Update(5,std::string("three"));
    }

    void Modify2(trillek::RewindableMap<unsigned int,std::string,int64_t,50>& data) {
        data.Update(2,std::string("three"));
        data.Update(3,std::string("two"));
        data.Update(5,std::string("one"));
    }

    void Delete(unsigned int id) {
        rmap.Remove(id);
    }

    void Add() {
        rmap.Insert(1, "six");
    }

    void Commit() {
        rmap.Commit(0);
        Modify(rmap);
        Delete(4);
        rmap.Commit(100);
        Modify2(rmap);
        rmap.Insert(4, std::string("six"));
        rmap.Commit(200);
    }
protected:
    std::map<unsigned int, std::string> refmap0;
    std::map<unsigned int, std::string> refmap100;
    std::map<unsigned int, std::string> refmap200;

    trillek::RewindableMap<unsigned int,std::string,int64_t,50> rmap;
};

using trillek::RewindableMap;

namespace trillek {
    TEST_F(RewindableMapTest, Add) {
        for (auto i = 1; i <= 5; ++i) {
            EXPECT_TRUE(rmap.Map().at(i) == refmap0.at(i));
        }
    }
    TEST_F(RewindableMapTest, Update) {
        Modify(rmap);

        EXPECT_TRUE(rmap.Map().at(2) == std::string("one"));
    }
    TEST_F(RewindableMapTest, Delete) {
        Delete(4);
        std::string ret;

        EXPECT_THROW(ret = rmap.Map().at(4), std::out_of_range);
    }
    TEST_F(RewindableMapTest, AddExisting) {
        Add();

        EXPECT_TRUE(rmap.Map().at(1) == std::string("one"));
    }
    TEST_F(RewindableMapTest, Commit) {
        Commit();

        for (auto& entry : refmap200) {
            ASSERT_TRUE(rmap.Map().at(entry.first) == entry.second);
        }
    }
    TEST_F(RewindableMapTest, Pull1) {
        Commit();
        int64_t last_confirmed = 0;
        std::shared_ptr<int64_t> rebase;

        auto ret = rmap.Pull(100, last_confirmed, rebase);

        EXPECT_TRUE(last_confirmed == 100);
        EXPECT_FALSE(rebase);
    }
    TEST_F(RewindableMapTest, Pull2) {
        Commit();
        int64_t last_confirmed = 0;
        std::shared_ptr<int64_t> rebase;
        auto ret = rmap.Pull(200, last_confirmed, rebase);

        EXPECT_EQ(200,last_confirmed);
        EXPECT_FALSE(rebase);
    }
    TEST_F(RewindableMapTest, Pull3) {
        Commit();
        int64_t last_confirmed = 0;
        std::shared_ptr<int64_t> rebase;
        auto ret = rmap.Pull(200, last_confirmed, rebase);

        EXPECT_TRUE(last_confirmed == 200);
        EXPECT_FALSE(rebase);
    }
    TEST_F(RewindableMapTest, Rewind1) {
        Commit();
        auto tp = rmap.Checkout(100);
        std::string ret;

        EXPECT_TRUE(tp == 100);
        for (auto& entry : refmap100) {
            ASSERT_TRUE(rmap.Map().at(entry.first) == entry.second);
        }
        EXPECT_THROW(ret = rmap.Map().at(4), std::out_of_range);
    }
    TEST_F(RewindableMapTest, Rewind2) {
        Commit();
        auto tp = rmap.Checkout(0);

        EXPECT_TRUE(tp == 0);
        for (auto& entry : refmap0) {
            ASSERT_TRUE(rmap.Map().at(entry.first) == entry.second);
        }
    }
    TEST_F(RewindableMapTest, AddWhileRewind) {
        Commit();
        auto tp = rmap.Checkout(0);
        EXPECT_TRUE(tp == 0);
        rmap.Insert(6, std::string("six"));
        std::string ret;

        EXPECT_THROW(ret = rmap.Map().at(6), std::out_of_range);
    }
    TEST_F(RewindableMapTest, UpdateWhileRewind) {
        Commit();
        auto tp = rmap.Checkout(0);
        EXPECT_TRUE(tp == 0);
        rmap.Update(1, std::string("six"));

        EXPECT_TRUE(rmap.Map().at(1) == refmap0.at(1));
    }
    TEST_F(RewindableMapTest, DeleteWhileRewind) {
        Commit();
        auto tp = rmap.Checkout(0);
        EXPECT_TRUE(tp == 0);
        rmap.Remove(1);
        std::string ret;

        ASSERT_NO_THROW(ret = rmap.Map().at(1));
        EXPECT_TRUE(rmap.Map().at(1) == refmap0.at(1));
    }
    TEST_F(RewindableMapTest, Forward1) {
        Commit();
        auto tp = rmap.Checkout(0);
        EXPECT_TRUE(tp == 0);
        tp = rmap.Checkout(100);
        EXPECT_TRUE(tp == 100);
        std::string ret;

        for (auto& entry : refmap100) {
            ASSERT_TRUE(rmap.Map().at(entry.first) == entry.second);
        }
        EXPECT_THROW(ret = rmap.Map().at(4), std::out_of_range);

        tp = rmap.Checkout(200);
        EXPECT_TRUE(tp == 200);
        for (auto& entry : refmap200) {
            ASSERT_TRUE(rmap.Map().at(entry.first) == entry.second);
        }
    }
    TEST_F(RewindableMapTest, Forward2) {
        Commit();
        auto tp = rmap.Checkout(0);
        EXPECT_TRUE(tp == 0);
        tp = rmap.Checkout(200);
        EXPECT_TRUE(tp == 200);
        std::string ret;

        for (auto& entry : refmap200) {
            ASSERT_TRUE(rmap.Map().at(entry.first) == entry.second);
        }
    }
    TEST_F(RewindableMapTest, Push) {
        Commit();
        int64_t last = -1;
        std::shared_ptr<int64_t> rebase;
        auto history = rmap.Pull(200, last, rebase);
        EXPECT_FALSE(rebase);
        EXPECT_TRUE(last == 200);

        trillek::RewindableMap<unsigned int,std::string,int64_t,50> dest;
        auto ret = dest.Push(history.first, history.second);
        EXPECT_TRUE(ret == 200);

        for (auto& entry : refmap200) {
            ASSERT_TRUE(dest.Map().at(entry.first) == entry.second);
        }
    }
    TEST_F(RewindableMapTest, Rebase) {
        Commit();
        int64_t last = -1;
        std::shared_ptr<int64_t> rebase;
        auto history = rmap.Pull(200, last, rebase);
        EXPECT_FALSE(rebase);
        EXPECT_TRUE(last == 200);

        trillek::RewindableMap<unsigned int,std::string,int64_t,50> dest;
        dest.Insert(1, "one from origin");
        auto tp = dest.Commit(0);
        EXPECT_TRUE(tp == 0);

        // simulate a consumer
        int64_t last2 = -1;
        auto history2 = dest.Pull(200, last2, rebase);
        EXPECT_FALSE(rebase);

        auto ret = dest.Push(history.first, history.second);
        EXPECT_TRUE(ret == 200);
        for (auto& entry : refmap200) {
            EXPECT_TRUE(dest.Map().at(entry.first) == entry.second);
        }

        last2 = 100;
        history2 = dest.Pull(200, last2, rebase);
        EXPECT_TRUE(rebase != false);
        EXPECT_TRUE(*rebase == -1);
    }
    TEST_F(RewindableMapTest, Rebase2) {
        // prepare a first map with data
        Commit();
        int64_t last = -1;
        std::shared_ptr<int64_t> rebase;
        // Pull the modifications
        auto history = rmap.Pull(200, last, rebase);
        EXPECT_FALSE(rebase);
        EXPECT_TRUE(last == 200);

        // prepare a second map with some content
        trillek::RewindableMap<unsigned int,std::string,int64_t,50> dest;
        auto tp = dest.Commit(0);
        dest.Insert(1, "one from origin");
        tp = dest.Commit(100);
        EXPECT_TRUE(tp == 100);
        EXPECT_TRUE(dest.Map().at(1) == "one from origin");

        // simulate a consumer that pulls the modifications from the second map
        int64_t last2 = -1;
        auto history2 = dest.Pull(100, last2, rebase);
        EXPECT_EQ(100, last2);
        EXPECT_FALSE(rebase);

        // push modifications from the first map in the second map
        auto ret = dest.Push(history.first, history.second);
        EXPECT_TRUE(ret == 200);
        for (auto& entry : refmap200) {
            EXPECT_TRUE(dest.Map().at(entry.first) == entry.second);
        }

        // simulate a consumer that pulls the modifications from the second map
        history2 = dest.Pull(200, last2, rebase);
        EXPECT_TRUE(last2 == 200);
        // check that a rebase has been raised with index -1
        EXPECT_TRUE(rebase != false);
        EXPECT_TRUE(*rebase == -1);

        // prepare a 3rd map and push modifications that the consumer got
        trillek::RewindableMap<unsigned int,std::string,int64_t,50> dest2;
        ret = dest2.Push(history2.first, history2.second);
        EXPECT_TRUE(ret == 200);
        // The 3rd map is identic to the original
        for (auto& entry : refmap200) {
            EXPECT_TRUE(dest2.Map().at(entry.first) == entry.second);
        }
    }
    TEST_F(RewindableMapTest, Rebase3) {
        // prepare a first map with data
        Commit();
        int64_t last = -1;
        std::shared_ptr<int64_t> rebase;
        // Pull the modifications
        auto history = rmap.Pull(200, last, rebase);
        EXPECT_TRUE(history.first.cbegin()->first == 0);
        EXPECT_TRUE((--history.first.cend())->first == 200);
        EXPECT_FALSE(rebase);
        EXPECT_TRUE(last == 200);

        // prepare a second map with some content
        trillek::RewindableMap<unsigned int,std::string,int64_t,50> dest;
        auto tp = dest.Commit(0);
        dest.Insert(1, "one from origin");
        tp = dest.Commit(100);
        EXPECT_TRUE(tp == 100);
        EXPECT_TRUE(dest.Map().at(1) == "one from origin");
        dest.Insert(2, "two from origin");
        tp = dest.Commit(200);
        EXPECT_TRUE(tp == 200);
        EXPECT_TRUE(dest.Map().at(2) == "two from origin");
        dest.Update(1, "three from origin");
        tp = dest.Commit(300);
        EXPECT_TRUE(tp == 300);
        EXPECT_TRUE(dest.Map().at(1) == "three from origin");

        // simulate a consumer that pulls the modifications from the second map
        int64_t last2 = -1;
        auto history2 = dest.Pull(200, last2, rebase);
        EXPECT_TRUE(history2.first.cbegin()->first == 0);
        EXPECT_TRUE((--history2.first.cend())->first == 200);
        EXPECT_EQ(last2,200);
        EXPECT_FALSE(rebase);
        // prepare a 3rd map and push modifications that the consumer got
        trillek::RewindableMap<unsigned int,std::string,int64_t,50> dest2;
        auto ret = dest2.Push(history2.first, history2.second);
        EXPECT_TRUE(ret == 200);

        // push modifications from the first map in the second map
        ret = dest.Push(history.first, history.second);
        EXPECT_TRUE(ret == 300);
        auto entry = refmap200.cbegin();
        EXPECT_TRUE(dest.Map().at(1) == "three from origin");
        for (++entry; entry != refmap200.cend(); ++entry) {
            EXPECT_TRUE(dest.Map().at(entry->first) == entry->second);
        }

        // simulate a consumer that pulls the modifications from the second map
        history2 = dest.Pull(300, last2, rebase);
        EXPECT_TRUE(history2.first.cbegin()->first == 0);
        EXPECT_TRUE((--history2.first.cend())->first == 300);
        EXPECT_TRUE(last2 == 300);
        // the rebase has been made at 300 > 200, so not notified
        ASSERT_FALSE(rebase == false);
        EXPECT_TRUE(*rebase == -1);

        ret = dest2.Push(history2.first, history2.second);
        EXPECT_TRUE(ret == 300);

        Modify(rmap);
        rmap.Commit(300);
        Modify2(rmap);
        rmap.Commit(400);
        history = rmap.Pull(400, last, rebase);
        EXPECT_TRUE(history.first.cbegin()->first == 300);
        EXPECT_TRUE((--history.first.cend())->first == 400);
        EXPECT_FALSE(rebase);
        ret = dest.Push(history.first, history.second);
        EXPECT_TRUE(ret == 400);
        EXPECT_EQ(rmap.Map().at(1),dest.Map().at(1));
        history2 = dest.Pull(400, last2, rebase);
        EXPECT_TRUE(history2.first.cbegin()->first == 300);
        EXPECT_TRUE((--history2.first.cend())->first == 400);
        EXPECT_TRUE(last2 == 400);
        EXPECT_FALSE(rebase == false);
        EXPECT_TRUE(*rebase == 299);
        ret = dest2.Push(history2.first, history2.second);
        EXPECT_TRUE(ret == 400);
        EXPECT_EQ(rmap.Map().at(1),dest2.Map().at(1));
    }
}
#endif // REWINDABLE_MAP_TEST_HPP_INCLUDED
