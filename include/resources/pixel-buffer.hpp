#ifndef PIXELBUFFER_HPP_INCLUDED
#define PIXELBUFFER_HPP_INCLUDED

#include "systems/resource-system.hpp"
#include <mutex>

namespace trillek {
namespace resource {

enum class ImageColorMode : uint32_t {
    UNKNOWN_MODE = 0xffffffffu,
    MONOCHROME = 0,
    MONOCHROME_A = 4,
    COLOR_RGB = 2,
    COLOR_RGBA = 6,
};

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
    virtual ~PixelBuffer();

    PixelBuffer(const PixelBuffer &) = delete;
    PixelBuffer & operator=(const PixelBuffer &) = delete;

    PixelBuffer(PixelBuffer &&);
    PixelBuffer & operator=(PixelBuffer &&);

    /** \brief Construct a new pixel buffer.
     * \param width the width of the buffer in pixels
     * \param height the height of the buffer in pixels
     * \param bitspersample the number of bits in a single channel, usually 8
     * \param mode greyscale (Monochrome) or color (RGB), and whether there is alpha or no alpha
     */
    PixelBuffer(uint32_t width, uint32_t height, uint32_t bitspersample, ImageColorMode mode);

    /** \brief Create a copy of a pixel buffer.
     * \param pbuf the pixel buffer to copy from
     * \return true on success
     */
    bool CreateCopy(const PixelBuffer & pbuf);

    /** \brief Create a copy of a pixel buffer in a different format.
     * \param pbuf the pixel buffer to copy from
     * \param bitspersample the number of bits in a single channel, usually 8
     * \param mode greyscale (Monochrome) or color (RGB), and whether there is alpha or no alpha
     * \return true on success
     */
    bool CreateCopy(const PixelBuffer & pbuf, uint32_t bitspersample, ImageColorMode mode);

    /** \brief Create a new pixel buffer, replacing the old buffer (if any).
     * \param width the width of the buffer in pixels
     * \param height the height of the buffer in pixels
     * \param bitspersample the number of bits in a single channel, usually 8
     * \param mode greyscale (Monochrome) or color (RGB), and whether there is alpha or no alpha
     * \return true on success
     */
    bool Create(uint32_t width, uint32_t height, uint32_t bitspersample, ImageColorMode mode);

    bool IsDirty() const;
    /** \brief Mark dirty */
    void Invalidate();
    /** \brief Mark not dirty */
    void Validate();

    /**
     * \brief Returns a pointer to the base address of image data for reading.
     * This function is intended for loading pixel data from the buffer.
     * \return uint8_t * base image address or nullptr if invalid.
     */
    const uint8_t * GetBlockBase() const;

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

    uint32_t Width() const { return imagewidth; }
    uint32_t Height() const { return imageheight; }
    uint32_t Pitch() const { return bufferpitch; }
    uint32_t PixelSize() const { return imagepixelsize; }
    ImageColorMode GetFormat() const { return imagemode; }

    // output a PPM image to stderr or file as a debug feature
    void PPMDebug();
    void PPMDebug(const char *);

    // meta data, used by some formats such as cursors
    uint32_t image_x;
    uint32_t image_y;
    std::vector<Property> meta;

protected:
    uint32_t imagewidth;
    uint32_t imageheight;
    uint32_t bufferpitch; /// number of bytes to move vertical 1 raster line
    uint32_t imagepixelsize;
    uint32_t imagebitdepth;
    ImageColorMode imagemode;

    bool dirty;

    std::unique_ptr<uint8_t[]> blockptr;
    std::mutex writelock;
};

} // resource

namespace reflection {

template <> inline const char* GetTypeName<resource::PixelBuffer>() { return "Image"; }
template <> inline unsigned int GetTypeID<resource::PixelBuffer>() { return 1003; }

} // reflection
} // trillek
#endif
