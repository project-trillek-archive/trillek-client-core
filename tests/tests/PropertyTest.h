#ifndef PROPERTY_TEST_H_INCLUDED
#define PROPERTY_TEST_H_INCLUDED

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"
#include <string>

#include "property.hpp"

namespace trillek {
    // Basic move and copy tests with POD int
    TEST(PropertyTest, PropertyName) {
        const std::string name = "PropertyTestName";
        int testINT = 10;
        Property p(name, testINT);
        EXPECT_EQ(name, p.GetName());
    }
    TEST(PropertyTest, PropertyMovePOD) {
        const std::string name = "PropertyTestName";
        int testINT = 10;
        Property p(name, testINT);
        Property moved_P = std::move(p);
        EXPECT_EQ(10, moved_P.Get<int>());
    }
    TEST(PropertyTest, PropertyCopyPOD) {
        const std::string name = "PropertyTestName";
        int testINT = 10;
        Property p(name, testINT);
        Property copied_P(p);
        EXPECT_EQ(10, copied_P.Get<int>());
    }

    // Testing move and copy with complex type
    TEST(PropertyTest, PropertyMoveComplex) {
        const std::string name = "PropertyTestName";
        std::vector<int> testVec;
        testVec.push_back(10);
        Property p(name, testVec);
        Property moved_P = std::move(p);
        EXPECT_EQ(10, moved_P.Get<std::vector<int>>()[0]);
    }
    TEST(PropertyTest, PropertyCopyComplex) {
        const std::string name = "PropertyTestName";
        std::vector<int> testVec;
        testVec.push_back(10);
        Property p(name, testVec);
        Property copied_P(p);
        EXPECT_EQ(10, copied_P.Get<std::vector<int>>()[0]);
    }

    // Advanced move and copy tests using pointers
    TEST(PropertyTest, PropertyMovePTR) {
        const std::string name = "PropertyTestName";
        int* testINT = new int();
        *testINT = 10;
        Property p(name, testINT);
        Property moved_P = std::move(p);
        EXPECT_EQ(10, *moved_P.Get<int*>());
        delete testINT;
    }
    TEST(PropertyTest, PropertyCopyPTR) {
        const std::string name = "PropertyTestName";
        int* testINT = new int();
        *testINT = 10;
        Property p(name, testINT);
        Property copied_P(p);
        EXPECT_EQ(10, *copied_P.Get<int*>());
        delete testINT;
    }

    // Testing get without a different type than the set
    TEST(PropertyTest, PropertyGetDifferentType_Exception) {
        const std::string name = "PropertyTestName";
        int testINT = 10;
        Property p2(name, testINT);
        /*try {
            std::vector<int> bad = p2.Get<std::vector<int>>(); // DOES a SEG FAULT!
            FAIL();
        }
        catch (...) {
            SUCCEED();
        }*/
    }
    TEST(PropertyTest, PropertyGetPTRDifferentType_Exception) {
        const std::string name = "PropertyTestName";
        int* testINT = new int();
        *testINT = 10;
        Property p2(name, testINT);
        /*try {
            std::vector<int> bad = p2.Get<std::vector<int>>(); // DOES a SEG FAULT
            FAIL();
        }
        catch (...) {
            SUCCEED();
        }*/
        delete testINT;
    }
}  // namespace

#endif
