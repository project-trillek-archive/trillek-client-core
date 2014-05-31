#ifndef IMAGELOADERTEST_INCLUDED
#define IMAGELOADERTEST_INCLUDED

#include "gtest/gtest.h"
#include "util/imageloader.hpp"
#include "resources/pixel-buffer.hpp"
#include <fstream>
#include <iostream>

namespace trillek {
namespace resource {

TEST(ImageLoaderTest, CreatePixelBuffer) {
    PixelBuffer image;
    EXPECT_TRUE(image.Create(200, 200, 8, ImageColorMode::COLOR_RGBA));
    EXPECT_EQ(200 * 4, image.Pitch());
    EXPECT_NE(nullptr, image.GetBlockBase());
}

TEST(ImageLoaderTest, PortableNetworkGraphic1) {
    PixelBuffer image;
    util::void_er stat;
    std::ifstream file;

    file.open("assets/T1.png", std::ios::in | std::ios::binary);
    EXPECT_TRUE(file.is_open());
    if(file.is_open()) {
        util::StdInputStream insf(file);
        stat = png::Load(insf, image);
        EXPECT_FALSE(stat);
        if(stat) {
            std::cout << "Reason: " << stat.Text() << " - " << stat.line << '\n';
        }
        else {
            image.PPMDebug("T1.ppm");
        }
    }
    file.close();
}
TEST(ImageLoaderTest, PortableNetworkGraphic2) {
    PixelBuffer image;
    util::void_er stat;
    std::ifstream file;
    file.open("assets/T2.png", std::ios::in | std::ios::binary);
    EXPECT_TRUE(file.is_open());
    if(file.is_open()) {
        util::StdInputStream insf(file);
        stat = png::Load(insf, image);
        EXPECT_FALSE(stat);
        if(stat) {
            std::cout << "Reason: " << stat.Text() << " - " << stat.line << '\n';
        }
        else {
            image.PPMDebug("T2.ppm");
        }
    }
    file.close();
}
TEST(ImageLoaderTest, PortableNetworkGraphic3) {
    PixelBuffer image;
    util::void_er stat;
    std::ifstream file;
    file.open("assets/T3.png", std::ios::in | std::ios::binary);
    EXPECT_TRUE(file.is_open());
    if(file.is_open()) {
        util::StdInputStream insf(file);
        stat = png::Load(insf, image);
        EXPECT_FALSE(stat);
        if(stat) {
            std::cout << "Reason: " << stat.Text() << " - " << stat.line << '\n';
        }
        else {
            image.PPMDebug("T3.ppm");
        }
    }
    file.close();
}
TEST(ImageLoaderTest, PortableNetworkGraphic4) {
    PixelBuffer image;
    util::void_er stat;
    std::ifstream file;
    file.open("assets/T4.png", std::ios::in | std::ios::binary);
    EXPECT_TRUE(file.is_open());
    if(file.is_open()) {
        util::StdInputStream insf(file);
        stat = png::Load(insf, image);
        EXPECT_FALSE(stat);
        if(stat) {
            std::cout << "Reason: " << stat.Text() << " - " << stat.line << '\n';
        }
        else {
            image.PPMDebug("T4.ppm");
        }
    }
    file.close();
}

}
}

#endif
