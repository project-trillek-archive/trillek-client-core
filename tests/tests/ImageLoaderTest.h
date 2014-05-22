#ifndef IMAGELOADERTEST_INCLUDED
#define IMAGELOADERTEST_INCLUDED

#include "gtest/gtest.h"
#include "util/imageloader.hpp"
#include "resources/pixel-buffer.hpp"
#include <fstream>
#include <iostream>

namespace trillek {
namespace resource {

TEST(ImageLoaderTest, PortableNetworkGraphic) {
    PixelBuffer image;
    util::void_er stat;
    std::ifstream file;

    file.open("T1.png", std::ios::in | std::ios::binary);
    EXPECT_TRUE(file.is_open());
    if(file.is_open()) {
        util::StdInputStream insf(file);
        stat = LoadPNG(insf, image);
        EXPECT_FALSE(stat);
        if(stat) {
            std::cerr << "Reason: " << stat.error_text << '\n';
        }
    }
    file.close();
    file.open("T2.png", std::ios::in | std::ios::binary);
    EXPECT_TRUE(file.is_open());
    if(file.is_open()) {
        util::StdInputStream insf(file);
        stat = LoadPNG(insf, image);
        EXPECT_FALSE(stat);
        if(stat) {
            std::cerr << "Reason: " << stat.error_text << '\n';
        }
    }
    file.close();
}

}
}

#endif
