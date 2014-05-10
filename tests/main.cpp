#include <gtest/gtest.h>

#include "tests/PropertyTest.h"
#include "tests/AtomicQueueTest.h"
#include "tests/AtomicMapTest.h"
#include "tests/ResourceSystemTest.h"
#include "tests/UtilityTest.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
