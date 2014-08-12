#ifndef IMAGELOADERTEST_INCLUDED
#define IMAGELOADERTEST_INCLUDED

#include "gtest/gtest.h"
#include "util/imageloader.hpp"
#include "resources/pixel-buffer.hpp"
#include <fstream>
#include <iostream>

// helpful for debugging images
//#define IMAGELOADER_WRITE_PPM

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

    file.open("assets/tests/T1.png", std::ios::in | std::ios::binary);
    EXPECT_TRUE(file.is_open());
    if(file.is_open()) {
        util::StdInputStream insf(file);
        stat = png::Load(insf, image);
        EXPECT_FALSE(stat) << stat.Text() << " - " << stat.line << '\n';
#ifdef IMAGELOADER_WRITE_PPM
        if(!stat) {
            image.PPMDebug("T1.ppm");
        }
#endif
    }
    file.close();
}
TEST(ImageLoaderTest, PortableNetworkGraphic2) {
    PixelBuffer image;
    util::void_er stat;
    std::ifstream file;
    file.open("assets/tests/T2.png", std::ios::in | std::ios::binary);
    EXPECT_TRUE(file.is_open());
    if(file.is_open()) {
        util::StdInputStream insf(file);
        stat = png::Load(insf, image);
        EXPECT_FALSE(stat) << stat.Text() << " - " << stat.line << '\n';
#ifdef IMAGELOADER_WRITE_PPM
        if(!stat) {
            image.PPMDebug("T2.ppm");
        }
#endif
    }
    file.close();
}
TEST(ImageLoaderTest, PortableNetworkGraphic3) {
    PixelBuffer image;
    util::void_er stat;
    std::ifstream file;
    file.open("assets/tests/T3.png", std::ios::in | std::ios::binary);
    EXPECT_TRUE(file.is_open());
    if(file.is_open()) {
        util::StdInputStream insf(file);
        stat = png::Load(insf, image);
        EXPECT_FALSE(stat) << stat.Text() << " - " << stat.line << '\n';
#ifdef IMAGELOADER_WRITE_PPM
        if(!stat) {
            image.PPMDebug("T3.ppm");
        }
#endif
    }
    file.close();
}
TEST(ImageLoaderTest, PortableNetworkGraphic4) {
    PixelBuffer image;
    util::void_er stat;
    std::ifstream file;
    file.open("assets/tests/T4.png", std::ios::in | std::ios::binary);
    EXPECT_TRUE(file.is_open());
    if(file.is_open()) {
        util::StdInputStream insf(file);
        stat = png::Load(insf, image);
        EXPECT_FALSE(stat) << stat.Text() << " - " << stat.line << '\n';
#ifdef IMAGELOADER_WRITE_PPM
        if(!stat) {
            image.PPMDebug("T4.ppm");
        }
#endif
    }
    file.close();
}
TEST(ImageLoaderTest, PortableNetworkGraphic5) {
    PixelBuffer image;
    util::void_er stat;
    std::ifstream file;
    file.open("assets/tests/T5.png", std::ios::in | std::ios::binary);
    EXPECT_TRUE(file.is_open());
    if(file.is_open()) {
        util::StdInputStream insf(file);
        stat = png::Load(insf, image);
        EXPECT_FALSE(stat) << stat.Text() << " - " << stat.line << '\n';
#ifdef IMAGELOADER_WRITE_PPM
        if(!stat) {
            image.PPMDebug("T5.ppm");
        }
#endif
    }
    file.close();
}
TEST(ImageLoaderTest, PortableNetworkGraphic6) {
    PixelBuffer image;
    util::void_er stat;
    std::ifstream file;
    file.open("assets/tests/T6.png", std::ios::in | std::ios::binary);
    EXPECT_TRUE(file.is_open());
    if(file.is_open()) {
        util::StdInputStream insf(file);
        stat = png::Load(insf, image);
        EXPECT_FALSE(stat) << stat.Text() << " - " << stat.line << '\n';
#ifdef IMAGELOADER_WRITE_PPM
        if(!stat) {
            image.PPMDebug("T6.ppm");
        }
#endif
    }
    file.close();
}

}
}

#endif
