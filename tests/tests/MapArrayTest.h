#ifndef MAPARRAYTEST_H_INCLUDED
#define MAPARRAYTEST_H_INCLUDED

#include "map-array.hpp"
#include <stdexcept>
#include <random>
#include <chrono>
#include <memory>


#include "gtest/gtest.h"

class MapArrayTest: public ::testing::Test {
protected:
    virtual void SetUp() {};

    void FillRandom(trillek::MapArray<double>& v, const size_t size) {
        for (size_t i = 0; i < size; i++) {
            auto t = next()/10000;
            v[i] = t;
            witness.push_back(t);
        }
    }

    double next() {
        return ((double) (random() - (random.max()>> 1)));
    }

    trillek::MapArray<double> wp256;
    double original = 1.0;
    std::vector<double> witness;
    std::vector<std::weak_ptr<double>> stored;
    unsigned long int seed = (unsigned long int) std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine random{seed};
};

using trillek::MapArray;

namespace trillek {
TEST_F(MapArrayTest, MapArrayBasic) {
    ASSERT_NO_THROW(wp256[0] = original) << "Failed to create element";
    EXPECT_EQ(wp256.at(0), original) << "Element retrieved is different";
    EXPECT_NO_THROW(wp256.clear(0)) << "Could not remove element";
}

TEST_F(MapArrayTest, MapArrayFill) {
    auto index = 256;
    FillRandom(wp256, index);
    for (unsigned int i = 0; i < index ; i++) {
        EXPECT_NO_THROW(wp256.at(i)) << "Could not access element " << i;
        EXPECT_EQ(wp256.at(i), witness[i]) << "Element " << i << " is not equal";
    }
    EXPECT_THROW(wp256.at(256), std::out_of_range) << "Access to uninitialized chunk should throw";
}

TEST_F(MapArrayTest, MapArrayOperations) {
    auto index = 256;
    FillRandom(wp256, index);
    index++;
    ASSERT_NO_THROW(wp256[index] = original) << "Failed to create element";
    EXPECT_EQ(wp256.at(index), original) << "Element retrieved is different";
    EXPECT_NO_THROW(wp256.clear(index)) << "Could not remove element";
}

TEST_F(MapArrayTest, MapArrayCopyElement) {
    auto index = 256;
    FillRandom(wp256, index);
    index++;
    wp256[index] = wp256.at(12);
    ASSERT_EQ(wp256.at(index), wp256.at(12)) << "Failed to copy element";
}

TEST_F(MapArrayTest, MapArrayMoveElement) {
    auto index = 256;
    FillRandom(wp256, index);
    index++;
    wp256[index] = 1.0;
    const double x = wp256.at(index);
    ASSERT_EQ(x, 1.0) << "Failed to move element";
}
}

#endif // MAPARRAYTEST_H_INCLUDED
