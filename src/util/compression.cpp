
#include "util/utiltype.hpp"
#include "util/compression.hpp"

namespace trillek {
namespace util {
namespace algorithm {

BitStreamDecoder::BitStreamDecoder() {
    indata = DataString();
    inpos = 0;
    num_bits = 0;
    bit_buffer = 0;
    num_unused = 0;
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
        return ErrorReturn<uint8_t>(0, 1, "Not enough data");
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
            return void_er(1, "Not enough data");
        }
    } else {
        return void_er(0);
    }
}
void_er BitStreamDecoder::Require(uint32_t n) {
    void_er ret;
    if(n > 24) {
        // get the number bytes that would need to be fetched
        n -= num_bits; // excluding what we have fetched
        uint32_t nbytes = n >> 3;
        if(n & 0x7) nbytes++; // non-multiples of 8 bits
        if(indata.length() - inpos < nbytes) {
            return void_er(1, "Not enough data");
        }
    } else {
        while(num_bits < n) {
            ret = LoadByte();
            if(ret) return ret;
        }
    }
    return void_er();
}
void_er BitStreamDecoder::AlignToByte() {
    if(num_bits & 0x7) {
        num_unused += num_bits & 0x7;
        GetBits(num_bits & 0x7);
    }
    return void_er();
}
void_er BitStreamDecoder::LoadFull() {
    while(num_bits <= 24) {
        if(!(bit_buffer < 1u << num_bits)) {
            return void_er(-2, "Bit buffer corrupt");
        }
        if(inpos < indata.length()) {
            bit_buffer |= indata[inpos++] << num_bits;
            num_bits += 8;
            //return void_er(0);
        } else {
            return void_er(1, "Not enough data");
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

template<typename T, std::size_t max>
static void n_memarrayset(T (&mem)[max], const T value, std::size_t count) {
    if(count > max) count = max;
    for(std::size_t i = 0; i < count; i++) mem[i] = value;
}
template<typename T, std::size_t max>
static void n_memarrayset(T (&mem)[max], std::size_t start, const T value, std::size_t count) {
    if(start > max) return;
    std::size_t end = start + count;
    if(end > max) end = max;
    for(std::size_t i = start; i < end; i++) mem[i] = value;
}
// Inflate decoder Huffman functions are based on the public domain
// zlib decode - Sean Barrett, originally found in SOIL and modified for C++

void_er Huffman::Build(const uint8_t *sizelist, uint32_t num) {
    uint32_t i;
    uint16_t code;
    uint32_t next_code[16], sizes[17];

    // DEFLATE spec for generating codes
    n_memarrayset(sizes, 0u, sizeof(sizes));
    n_memarrayset(this->fast, (uint16_t)0xffffu, sizeof(this->fast));
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
        firstcode[i] = code;
        firstsymbol[i] = symbol;
        code += sizes[i];
        if(sizes[i]) {
            if(code - 1u >= (1u << i)) {
                return void_er(-5, "Bad codelengths");
            }
        }
        this->maxcode[i] = code << (16 - i); // preshift for inner loop
        code <<= 1;
        symbol += sizes[i];
    }
    this->maxcode[16] = 0x10000; // sentinel
    for(i = 0; i < num; ++i) {
        uint32_t codelen = sizelist[i];
        if(codelen) {
            uint16_t c;
            c = next_code[codelen] - firstcode[codelen] + firstsymbol[codelen];
            this->size[c] = (uint8_t)codelen;
            this->value[c] = (uint16_t)i;
            if(codelen <= 9) {
                uint32_t k = BitReverse(next_code[codelen], codelen);
                while(k < 512) {
                    fast[k] = c;
                    k += (1 << codelen);
                }
            }
            ++next_code[codelen];
        }
    }
    return void_er();
}

ErrorReturn<uint16_t> Inflate::HuffmanDecode(const Huffman& z) {
    uint32_t codesize;
    uint32_t b, k;
    void_er st = instream.Require(16);
    if(st) {
        return st.value<uint16_t>(0);
    }
    b = z.fast[instream.bit_buffer & 0x1FF];
    if(b < 0xffff) {
        if(!(b < 288)) {
            return ErrorReturn<uint16_t>(0, -4, "Index out of range");
        }
        codesize = z.size[b];
        instream.GetBits(codesize);
        return ErrorReturn<uint16_t>(z.value[b]);
    }

    // not resolved by fast table, so compute it
    k = BitReverse(instream.bit_buffer, 16);

    // validate
    for(codesize = 10; k >= z.maxcode[codesize] && codesize < 16; ++codesize);
    if(codesize == 16) {
        return ErrorReturn<uint16_t>(0, -3, "Code invalid");
    }

    b = (k >> (16 - codesize)) - z.firstcode[codesize] + z.firstsymbol[codesize];
    if(!(b < 288)) {
        return ErrorReturn<uint16_t>(0, -4, "Index out of range");
    }
    if(!z.size[b] == codesize) {
        return ErrorReturn<uint16_t>(0, -3, "Size table invalid");
    }
    instream.GetBits(codesize);
    return ErrorReturn<uint16_t>(z.value[b]);
}

void_er Inflate::UncompressedBlock() {
    return void_er();
}
void_er Inflate::DynamicBlock() {
    static const uint8_t ALPHABET_LENGTHS[19] = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };

    uint8_t lencodes[286 + 32 + 137];//padding for maximum single op
    uint8_t codelength_sizes[19];
    void_er ret;
    uint32_t i, n;

    if(readstate == InflateState::BLOCK_DYNAMIC) {
        ret = instream.Require(14);
        if(ret) return ret;
        s_num_literal_codes  = (ret = instream.GetBits(5)) + 257;
        if(ret) return ret;
        s_num_distance_codes = (ret = instream.GetBits(5)) + 1;
        if(ret) return ret;
        s_num_codelen_codes  = (ret = instream.GetBits(4)) + 4;
        if(ret) return ret;
        readstate = InflateState::BLOCK_DYNAMIC_CODELEN;
    }

    if(readstate == InflateState::BLOCK_DYNAMIC_CODELEN) {
        ret = instream.Require(s_num_codelen_codes * 3);
        if(ret) return ret;

        s_codelength = std::unique_ptr<Huffman>(new Huffman());

        n_memarrayset(codelength_sizes, (uint8_t)0, sizeof(codelength_sizes));
        for(i = 0; i < s_num_codelen_codes; ++i) {
            uint32_t s = (ret = instream.GetBits(3));
            if(ret) return ret;
            codelength_sizes[ALPHABET_LENGTHS[i]] = (uint8_t)s;
        }
        ret = s_codelength->Build(codelength_sizes, 19);
        if(ret) {
            readstate = InflateState::BAD_STREAM;
            return ret;
        }

        n = 0;
        readstate = InflateState::BLOCK_DYNAMIC_SYMREAD;
        s_lencodes = std::unique_ptr<InflateLengthCode>(new InflateLengthCode);
    }
    else {
        n = s_codenum;
    }

    while(n < s_num_literal_codes + s_num_distance_codes) {
        s_codenum = n;
        uint16_t sym, lc;
        if(readstate == InflateState::BLOCK_DYNAMIC_SYMREAD) {
            sym = (ret = HuffmanDecode(*s_codelength));
            if(ret) {
                return ret;
            }
            readstate = InflateState::BLOCK_DYNAMIC_SYMEXT;
        } else {
            sym = s_symbol;
        }
        if( !(sym >= 0u && sym < 19u) ) return void_er(-2, "Invalid decode");
        if(sym < 16u) {
            s_lencodes->v[n++] = (uint8_t)sym;
        }
        else if(sym == 16) {
            lc = (ret = instream.GetBits(2)) + 3;
            if(ret.error_code == 1) {
                s_symbol = sym;
                return ret;
            }
            n_memarrayset(s_lencodes->v, n, s_lencodes->v[n - 1], lc);
            n += lc;
        }
        else if(sym == 17) {
            lc = (ret = instream.GetBits(3)) + 3;
            if(ret.error_code == 1) {
                s_symbol = sym;
                return ret;
            }
            n_memarrayset(s_lencodes->v, n, (uint8_t)0, lc);
            n += lc;
        }
        else {
            //assert(sym == 18);
            lc = (ret = instream.GetBits(7)) + 11;
            if(ret.error_code == 1) {
                s_symbol = sym;
                return ret;
            }
            n_memarrayset(s_lencodes->v, n, (uint8_t)0, lc);
            n += lc;
        }
        readstate = InflateState::BLOCK_DYNAMIC_SYMREAD;
    }
    s_codelength.reset();
    if(n != s_num_literal_codes + s_num_distance_codes) {
        readstate = InflateState::BAD_STREAM;
        return void_er(-3, "Bad code lengths");
    }
    ret = lengthcodes.Build(s_lencodes->v, s_num_literal_codes);
    if(ret) {
        readstate = InflateState::BAD_STREAM;
        return ret;
    }
    ret = distancecodes.Build(s_lencodes->v + s_num_literal_codes, s_num_distance_codes);
    if(ret) {
        readstate = InflateState::BAD_STREAM;
        return ret;
    }

    s_lencodes.reset();

    return void_er();
}

// Constants used by deflate/inflate
static const uint32_t length_base[31] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
    15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
    67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0
};
static const uint32_t length_extra[31] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4,
    5, 5, 5, 5, 0, 0, 0
};
static const uint32_t dist_base[32] = {
    1, 2, 3, 4, 5, 7, 9, 13,
    17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073,
    4097, 6145, 8193, 12289, 16385, 24577, 0, 0
};
static const uint32_t dist_extra[32] = {
    0, 0, 0, 0, 1, 1, 2, 2,
    3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10,
    11, 11, 12, 12, 13, 13
};

void_er Inflate::HuffmanBlock() {
    uint16_t symbol;
    uint32_t length;
    uint32_t distance;
    unsigned char csym;
    void_er ret;

    while(!ret) {
        if(readstate == InflateState::BLOCK_DATA_SYMBOL) {
            symbol = s_symbol;
            readstate = InflateState::BLOCK_DATA;
        } else {
            symbol = (ret = HuffmanDecode(lengthcodes));
            if(ret) return ret;
        }
        if(symbol < 256) {
            csym = (unsigned char)symbol;
            outdata.append(&csym, 1);
        } else if(symbol == 256) {
            return void_er();
        } else {
            ret = instream.Require(18+16);
            if(ret) {
                s_symbol = symbol;
                readstate = InflateState::BLOCK_DATA_SYMBOL;
                return ret;
            }
            symbol -= 257;
            length = length_base[symbol];
            if(length_extra[symbol]) length += (ret = instream.GetBits(length_extra[symbol]));
            if(ret) {
                s_symbol = symbol;
                readstate = InflateState::BLOCK_DATA_SYMBOL;
                return ret;
            }
            symbol = (ret = HuffmanDecode(distancecodes));
            if(ret) {
                s_symbol = symbol;
                readstate = InflateState::BLOCK_DATA_SYMBOL;
                return ret;
            }
            distance = dist_base[symbol];
            if(dist_extra[symbol]) distance += (ret = instream.GetBits(dist_extra[symbol]));
            if(ret) {
                s_symbol = symbol;
                readstate = InflateState::BLOCK_DATA_SYMBOL;
                return ret;
            }
            outpos = outdata.length();
            if(outpos < distance) {
                return void_er(-2, "Invalid distance - output too short");
            }
            while(length--) {
                csym = outdata[outpos++ - distance];
                outdata.append(&csym, 1);
            }
        }
    }
    return void_er();
}

Inflate::Inflate() {
    outdata = DataString();
    outpos = 0;
    s_final = 0;
    s_symbol = 0;

    errored = false;

    readstate = InflateState::ZLIB_HEADER;
}

Inflate::~Inflate() {
}

bool Inflate::DecompressStart() {
    return false;
}

bool Inflate::DecompressEnd() {
    return false;
}

static const uint8_t default_length[288] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8
};
static const uint8_t default_distance[32] = {
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

bool Inflate::DecompressData(const DataString & data) {
    instream.AppendData(data);

    uint8_t final_flag, type;
    if(readstate == InflateState::ZLIB_HEADER) {
        instream.ReadByte();
        instream.ReadByte();
        checksum.Init();
        readstate = InflateState::BLOCK_HEADER;
    }

    error_state.error_code = 0;
    final_flag = 0;

    while(!final_flag && !error_state) {
        if(readstate == InflateState::BLOCK_HEADER) {
            error_state = instream.Require(3);
            if(error_state.error_code == 1) return false;
            final_flag = (error_state = instream.GetBits(1));
            s_final = final_flag;
            if(error_state) return true;
            type = (error_state = instream.GetBits(2));
            if(error_state) return true;
            switch(type) {
            case 0:
                readstate = InflateState::BLOCK_UNCOMPRESSED;
                break;
            case 1:
                // use fixed code lengths
                error_state = lengthcodes.Build(default_length, 288);
                if(error_state) return true;
                error_state = distancecodes.Build(default_distance, 32);
                if(error_state) return true;
                readstate = InflateState::BLOCK_DATA;
                break;
            case 2:
                readstate = InflateState::BLOCK_DYNAMIC;
                break;
            case 3:
                readstate = InflateState::BAD_STREAM;
                return true;
                break;
            }
        }
        else{
            final_flag = s_final;
        }
        switch(readstate) {
        case InflateState::BLOCK_UNCOMPRESSED:
            error_state = UncompressedBlock();
            if(error_state.error_code == 1) return false;
            if(error_state) return true;
            readstate = InflateState::BLOCK_HEADER;
            break;
        case InflateState::BLOCK_DYNAMIC:
            error_state = DynamicBlock();
            if(error_state.error_code == 1) return false;
            if(error_state) return true;
            readstate = InflateState::BLOCK_DATA;
            // fall through
        case InflateState::BLOCK_DATA:
        case InflateState::BLOCK_DATA_SYMBOL:
            error_state = HuffmanBlock();
            if(error_state.error_code == 1) return false;
            if(error_state) return true;
            readstate = InflateState::BLOCK_HEADER;
            break;
        default:
            return false;
            break;
        }
    }
    checksum.Update(outdata.data(), outdata.length());
    instream.AlignToByte();
    uint32_t checkval;
    checkval = instream.GetBits(8).value << 24;
    checkval |= instream.GetBits(8).value << 16;
    checkval |= instream.GetBits(8).value << 8;
    checkval |= instream.GetBits(8).value;
    checksum.Last();
    if(checksum.ldata != checkval) {
        return true;
    }
    return false;
}

bool Inflate::DecompressHasOutput() {
    return outdata.length() > 0;
}

DataString Inflate::DecompressGetOutput() {
    return outdata;
}

} // algorithm
} // util
} // trillek
