#ifndef COMPRESSION_HPP_INCLUDED
#define COMPRESSION_HPP_INCLUDED

#include <string>
#include <memory>
#include <stdint.h>

#include "util/utiltype.hpp"
#include "util/checksum.hpp"

namespace trillek {
namespace util {

typedef std::basic_string<
    unsigned char,
    std::char_traits<unsigned char>,
    std::allocator<unsigned char>
> DataString;

class CompressionMethod {
public:
    CompressionMethod() {}
    virtual ~CompressionMethod() {};
    virtual bool CompressStart() = 0;
    virtual bool CompressEnd() = 0;
    virtual bool CompressData(const DataString &) = 0;
    virtual bool CompressHasOutput() = 0;
    virtual DataString CompressGetOutput() = 0;
};
class DecompressionMethod {
public:
    DecompressionMethod() {}
    virtual ~DecompressionMethod() {};

    virtual bool DecompressStart() = 0;
    virtual bool DecompressEnd() = 0;
    virtual bool DecompressData(const DataString &) = 0;
    virtual bool DecompressHasOutput() = 0;
    virtual DataString DecompressGetOutput() = 0;
};

namespace algorithm {

    struct Huffman {
        uint16_t fast[512];
        uint16_t firstcode[16];
        uint32_t maxcode[17];
        uint16_t firstsymbol[16];
        uint8_t size[288];
        uint16_t value[288];

        void_er Build(const uint8_t *sizelist, uint32_t num);
    };

    struct BitStreamDecoder {
        DataString indata;
        DataString::size_type inpos;
        uint32_t num_bits;
        uint32_t bit_buffer;
        uint32_t num_unused;

        BitStreamDecoder();

        /** \brief put data into the stream
         */
        void_er AppendData(const DataString & in);

        /** \brief get a byte from the stream
         */
        ErrorReturn<uint8_t> ReadByte();

        /** \brief ensure bits are available
         */
        void_er Require(uint32_t n);

        void_er AlignToByte();

        // used for bit buffer filling
        void_er LoadByte();
        void_er LoadFull();

        /** \brief get bits out of the stream
         */
        ErrorReturn<uint32_t> GetBits(uint32_t n);

    };

    enum class InflateState : uint32_t {
        ZLIB_HEADER,
        BLOCK_HEADER,
        END_OF_STREAM,
        BAD_STREAM,
        BLOCK_UNCOMPRESSED,
        BLOCK_DYNAMIC,
        BLOCK_DYNAMIC_CODELEN,
        BLOCK_DYNAMIC_SYMREAD,
        BLOCK_DYNAMIC_SYMEXT,
        BLOCK_DATA,
        BLOCK_DATA_SYMBOL,
    };

    struct InflateLengthCode {
        uint8_t v[455];
    };

    class Inflate : public DecompressionMethod {
    public:
        Inflate();
        ~Inflate();
        bool DecompressStart();
        bool DecompressEnd();
        bool DecompressData(const DataString &);
        bool DecompressHasOutput();
        DataString DecompressGetOutput();

        const void_er& ErrorState() const { return error_state; }
    protected:
        ErrorReturn<uint16_t> HuffmanDecode(const Huffman&);
        void_er UncompressedBlock();
        void_er DynamicBlock();

        void_er HuffmanBlock();

        BitStreamDecoder instream;
        DataString outdata;
        DataString::size_type outpos;
        bool errored;
        void_er error_state;
        Huffman lengthcodes, distancecodes;
        InflateState readstate;
        uint8_t s_final;
        uint16_t s_symbol;
        Adler32 checksum;
        uint32_t s_num_literal_codes;
        uint32_t s_num_distance_codes;
        uint32_t s_num_codelen_codes;
        uint32_t s_codenum;

        std::unique_ptr<Huffman> s_codelength;
        // 286 + 32 + 137
        std::unique_ptr<InflateLengthCode> s_lencodes;
    };

} // algorithm

} // util
} // trillek

#endif
