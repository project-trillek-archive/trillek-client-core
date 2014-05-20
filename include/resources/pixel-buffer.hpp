#ifndef PIXELBUFFER_HPP_INCLUDED
#define PIXELBUFFER_HPP_INCLUDED

#include <istream>
#include "systems/ResourceSystem.h"
#include <fstream>
#include <mutex>

namespace trillek {
namespace resource {

enum class ImageColorMode : uint32_t {
    UNKNOWN_MODE = 0xffffffffu,
    MONOCHROME = 0,
    MONOCHROME_A = 4,
    INDEXED = 3,
    COLOR_RGB = 2,
    COLOR_RGBA = 6,
};

typedef std::allocator<unsigned char> image_allocator;

class PixelBuffer;

class PixelBuffer : public ResourceBase {
public:
    /**
     * \brief Returns a resource with the specified name.
     *
     * \param[in] const std::vector<Property> &properties The creation properties for the resource.
     * \return bool True if initialization finished with no errors.
     */
    bool Initialize(const std::vector<Property> &properties);

    PixelBuffer();
    PixelBuffer(const PixelBuffer &);
    PixelBuffer(PixelBuffer &&);
    PixelBuffer(uint32_t width, uint32_t height, uint32_t bitdepth, ImageColorMode mode);
    /** \brief Create a copy of a pixel buffer in a different format.
     */
    PixelBuffer(const PixelBuffer &, uint32_t bitdepth, ImageColorMode mode);
    PixelBuffer & operator=(const PixelBuffer &);
    PixelBuffer & operator=(PixelBuffer &&);

    bool Create(uint32_t width, uint32_t height, uint32_t bitdepth, ImageColorMode mode);

    bool IsDirty();

    /**
     * \brief Returns a pointer to the base address of image data for reading.
     * This function is intended for loading pixel data from the buffer.
     * \return uint8_t * base image address or nullptr if invalid.
     */
    const uint8_t * GetBlockBase();

    /**
     * \brief Locks image for writing and gets pointer to base address of image data.
     * This function is intended for image processing or filling the pixel buffer.
     * \return uint8_t * base image address or nullptr if invalid or lock failed.
     */
    uint8_t * LockWrite();
    /**
     * \brief Unlocks image after writing, must be called if LockWrite was successful.
     */
    void UnlockWrite();

private:
    uint32_t imagewidth;
    uint32_t imageheight;
    uint32_t bufferpitch; /// number of bytes to advance down 1 raster line
    uint32_t imagebitdepth;
    ImageColorMode imagemode;

    // meta data, used by some formats such as cursors
    uint32_t image_x;
    uint32_t image_y;

    bool dirty;

    std::unique_ptr<uint8_t[]> blockptr;
    std::mutex writelock;
};

} // resource
} // trillek
#endif
