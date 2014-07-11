#include <gtest/gtest.h>

#include "tests/PropertyTest.h"
#include "tests/AtomicQueueTest.h"
#include "tests/AtomicMapTest.h"
#include "tests/ResourceSystemTest.h"
#include "tests/UtilityTest.h"
#include "tests/DecompressorTest.h"
#include "tests/ImageLoaderTest.h"
#include "tests/transform-system-test.h"

size_t gAllocatedSize = 0;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
