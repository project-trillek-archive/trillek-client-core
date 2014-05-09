#ifndef COMPRESSION_HPP_INCLUDED
#define COMPRESSION_HPP_INCLUDED

#include <string>
#include <memory>
#include <stdint.h>

namespace trillek {
namespace util {

class CompressionMethod {
public:
    CompressionMethod() {}
    virtual ~CompressionMethod() {};
    virtual bool CompressStart() = 0;
    virtual bool CompressEnd() = 0;
    virtual bool CompressData(std::string) = 0;
    virtual bool CompressHasOutput() = 0;
    virtual std::string CompressGetOutput() = 0;
};
class DecompressionMethod {
public:
    DecompressionMethod() {}
    virtual ~DecompressionMethod() {};

    virtual bool DecompressStart() = 0;
    virtual bool DecompressEnd() = 0;
    virtual bool DecompressData(std::string) = 0;
    virtual bool DecompressHasOutput() = 0;
    virtual std::string DecompressGetOutput() = 0;
};

namespace algorithm {
    typedef struct
    {
        uint16_t fast[512];
        uint16_t firstcode[16];
        int maxcode[17];
        uint16_t firstsymbol[16];
        uint8_t size[288];
        uint16_t value[288];

        int Build(uint8_t *sizelist, int num);
    } Huffman;

    struct InflateState {
        std::string indata;
        unsigned long inpos;
        std::string outdata;
        unsigned long outpos;

        int num_bits;
        uint32_t bit_buffer;

        bool errored;
        char const * lasterror;

        Huffman length, distance;

        int readstate;

        InflateState();
        int FetchByte();
        uint32_t GetBits(int n);
    };

    class Inflate : public DecompressionMethod {
    public:
        Inflate();
        ~Inflate();
        bool DecompressStart();
        bool DecompressEnd();
        bool DecompressData(std::string);
        bool DecompressHasOutput();
        std::string DecompressGetOutput();
    protected:
        std::unique_ptr<InflateState> state;
    };
} // algorithm

} // util
} // trillek

#endif
