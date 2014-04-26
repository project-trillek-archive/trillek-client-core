#include <gtest/gtest.h>

#include "tests/AtomicQueueTest.h"
#include "tests/AtomicMapTest.h"

size_t gAllocatedSize = 0;

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();
	return ret;
}
