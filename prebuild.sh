#!/bin/bash
# Bash script to install all dependencies

echo "This script will try to download and install the necesary libs for Trillek engine on Ubuntu"
BUILD_PATH=$(pwd)

mkdir build

# Get GLEW and GLM
echo "Downloading GLEW and GLM"
sudo apt-get -q -y install libglew-dev libglm-dev

# Get Crypto++
#echo "Getting & compiling crypt++"
#git submodule update --init --depth 1 -- ./crypto++
#cd crypto++
#cmake CMakeLists.txt
#make
#cd $BUILD_PATH

# OpenAL, Vorbis and OGG
echo "OpenAL, Vorbis/OGG and Alure"
sudo apt-get -q -y install libogg-dev libvorbis-dev libopenal-dev libalure-dev

# Bullet
BULLET_DOUBLE="$(pkg-config bullet --cflags | grep DOUBLE_PRECISION )"
if [ -z "$BULLET_DOUBLE" ] ; then
    # Bullet not installed with Double precision
    echo "Installing and compiling bullet"
    cd /tmp/
    mkdir bullet
    cd bullet
    wget https://bullet.googlecode.com/files/bullet-2.82-r2704.tgz
    tar -xzxf bullet-2.82-r2704.tgz
    cd bullet-2.82-r2704
    cmake -DINSTALL_LIBS=on -DBUILD_SHARED_LIBS=on -DBUILD_DEMOS=off -DUSE_DOUBLE_PRECISION=on  .
    make
    sudo make install
else
    echo "Bullet installed with double precision"
fi
cd $BUILD_PATH

# install and build gtest
if [ ! -f /usr/src/gtest/build/libgtest.a ]; then
    echo "Instaling and compiling GTest"
    sudo apt-get -q -y install libgtest-dev
    cd /usr/src/gtest
    sudo cmake CMakeLists.txt
    sudo make
    GTEST_ROOT=/usr/src/gtest
    export GTEST_ROOT
else
    echo "GTest installed on /usr/src/gtest/"
fi
cd $BUILD_PATH

# Get GLFW and compile it !
GLFW_VERSION="$(pkg-config --modversion glfw3 | grep 3)"
if [ -z "$GLFW_VERSION" ] ; then  # Should be >= 3
    echo "Downloading and compiling GLFW"
    cd /tmp/
    sudo apt-get -qq install xorg-dev libglu1-mesa-dev
    git clone https://github.com/glfw/glfw.git
    cd glfw
    cmake CMAKE_CXX_FLAGS=-fPIC CMAKE_C_FLAGS=-fPIC cmake -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS:bool=true .
    make
    sudo make install
else
    echo "GLFW $GLFW_VERSION is installed "
fi
cd $BUILD_PATH

# Get RapidJSON
if [ ! -f /usr/local/include/rapidjson/rapidjson.h ]; then
    echo "Getting rapidjson"
    wget https://rapidjson.googlecode.com/files/rapidjson-0.11.zip
    unzip rapidjson-0.11.zip -d /tmp
    sudo cp -r /tmp/rapidjson/include /usr/local
else
    echo "Rapidjson was installed previusly on /usr/local/include/rapidjson/"
fi
cd $BUILD_PATH

echo "To build the engine, run build.sh"

