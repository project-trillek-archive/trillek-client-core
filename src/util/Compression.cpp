
#include "util/UtilType.hpp"
#include "util/Compression.hpp"

namespace trillek {
namespace util {
namespace algorithm {

BitStreamDecoder::BitStreamDecoder() {
    indata = DataString();
    inpos = 0;
    num_bits = 0;
    bit_buffer = 0;
}

void_er BitStreamDecoder::AppendData(const DataString & in)
{
    indata.append(in);
    return void_er();
}

ErrorReturn<uint8_t> BitStreamDecoder::ReadByte() {
    if(inpos < indata.length()) {
        return ErrorReturn<uint8_t>(indata[inpos++]);
    } else {
        return ErrorReturn<uint8_t>(0, -1, "Not enough data");
    }
}

void_er BitStreamDecoder::LoadByte() {
    if(num_bits <= 24) {
        if(!(bit_buffer < 1u << num_bits)) {
            return void_er(-2, "Bit buffer corrupt");
        }
        if(inpos < indata.length()) {
            bit_buffer |= indata[inpos++] << num_bits;
            num_bits += 8;
            return void_er(0);
        } else {
            return void_er(-1, "Not enough data");
        }
    } else {
        return void_er(0);
    }
}
void_er BitStreamDecoder::LoadFull() {
    while(num_bits <= 24) {
        if(!(bit_buffer < 1u << num_bits)) {
            return void_er(-2, "Bit buffer corrupt");
        }
        if(inpos < indata.length()) {
            bit_buffer |= indata[inpos++] << num_bits;
            num_bits += 8;
            return void_er(0);
        } else {
            return void_er(-1, "Not enough data");
        }
    }
    return void_er(0);
}

ErrorReturn<uint32_t> BitStreamDecoder::GetBits(uint32_t n) {
    if(n > 32) {
        return ErrorReturn<uint32_t>(0, -5, "Invalid input");
    }
    if(num_bits < n) {
        void_er st;
        st = LoadFull();
        if(num_bits < n && st) {
            return st.value<uint32_t>(0u);
        }
    }
    uint32_t rbits = bit_buffer & ((1u << n) - 1);
    bit_buffer >>= n;
    num_bits -= n;
    return ErrorReturn<uint32_t>(rbits);
}

// Inflate decoder Huffman functions are based on the public domain
// zlib decode - Sean Barrett, originally found in SOIL and modified for C++

void_er Huffman::Build(uint8_t *sizelist, uint32_t num) {
    uint32_t i;
    uint16_t code;
    uint32_t next_code[16], sizes[17];

    // DEFLATE spec for generating codes
    memset(sizes, 0, sizeof(sizes));
    memset(this->fast, 255, sizeof(this->fast));
    for(i = 0; i < num; ++i) {
        if(!(sizelist[i] <= 16)) return void_er(-5, "Bad sizelist");
        ++sizes[sizelist[i]];
    }
    sizes[0] = 0;
    for(i = 1; i < 16; ++i) {
        if(!(sizes[i] <= (1u << i))) {
            return void_er(-5, "Bad sizes");
        }
    }
    code = 0;
    uint16_t symbol = 0;
    for(i = 1; i < 16; ++i) {
        next_code[i] = code;
        this->firstcode[i] = code;
        this->firstsymbol[i] = symbol;
        code += sizes[i];
        if(sizes[i]) {
            if(code - 1u >= (1u << i)) return void_er(-5, "Bad codelengths");
        }
        this->maxcode[i] = code << (16 - i); // preshift for inner loop
        code <<= 1;
        symbol += sizes[i];
    }
    this->maxcode[16] = 0x10000; // sentinel
    for(i = 0; i < num; ++i) {
        uint32_t codelen = sizelist[i];
        if(codelen) {
            uint16_t c = next_code[codelen] - this->firstcode[codelen] + this->firstsymbol[codelen];
            this->size[c] = (uint8_t)codelen;
            this->value[c] = (uint16_t)i;
            if(codelen <= 9) {
                uint32_t k = BitReverse(next_code[codelen], codelen);
                while(k < 512) {
                    this->fast[k] = c;
                    k += (1 << codelen);
                }
            }
            ++next_code[codelen];
        }
    }
    return 1;
}

Inflate::Inflate() {
    outdata = DataString();
    outpos = 0;

    errored = false;

    readstate = InflateStateNumber::HEADER;
}

Inflate::~Inflate() {
}

bool Inflate::DecompressStart() {
    return false;
}

bool Inflate::DecompressEnd() {
    return false;
}

bool Inflate::DecompressData(DataString data) {
    instream.AppendData(data);
    uint32_t tmp;
    void_er status;

    tmp = status = instream.GetBits(5);
    if(status) {
        errored = true;
        return false;
    }
    error_state = status;
    return false;
}

bool Inflate::DecompressHasOutput() {
    return false;
}

DataString Inflate::DecompressGetOutput() {
    return DataString();
}

} // algorithm
} // util
} // trillek
