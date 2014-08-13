#!/bin/bash

if [ -f /usr/src/gtest/build/libgtest.a ]; then
    echo "Instaled GTest"
    GTEST_ROOT=/usr/src/gtest
    export GTEST_ROOT
    
    #mkdir build
    #cd build
    cmake -DTCC_BUILD_TESTS=True -DTCC_TEST_NETWORK_DISABLE=True -DTRILLEK_BUILD_DEPENDENCY=True .
else
    cmake -DTCC_BUILD_TESTS=False -DTCC_TEST_NETWORK_DISABLE=True -DTRILLEK_BUILD_DEPENDENCY=True .
fi

echo "Compiling with English error/warings"
LC_ALL=C make

