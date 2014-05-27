
#include "util/utiltype.hpp"
#include "util/compression.hpp"

namespace trillek {
namespace util {
namespace algorithm {

BitStreamDecoder::BitStreamDecoder() {
    indata = DataString();
    inlength = 0;
    inpos = 0;
    num_bits = 0;
    bit_buffer = 0;
    num_unused = 0;
}

void_er BitStreamDecoder::AppendData(const DataString & in)
{
    indata.append(in);
    inlength = indata.length();
    return void_er();
}

void BitStreamDecoder::Flush() {
    if(inpos == indata.length()) {
        indata.clear();
        inlength = indata.length();
        inpos = 0;
    }
}

uint32_t BitStreamDecoder::LoadByte() {
    if(num_bits <= 24) {
        if(!(bit_buffer < 1u << num_bits)) {
            return 2; // Bit buffer corrupt
        }
        if(inpos < inlength) {
            bit_buffer |= indata[inpos++] << num_bits;
            num_bits += 8;
            return 0;
        } else {
            return 1; // Not enough data
        }
    } else {
        return 0;
    }
}

bool BitStreamDecoder::Require(uint32_t n) {
    if(n > 24) {
        // get the number bytes that would need to be fetched
        n -= num_bits; // excluding what we have fetched
        uint32_t nbytes = n >> 3;
        if(n & 0x7) nbytes++; // non-multiples of 8 bits
        if(inlength - inpos < nbytes) {
            return true; // Not enough data
        }
    } else {
        if((bit_buffer & (0xffffffffu << num_bits))) {
            return true; // Bit buffer corrupt
        }
        while(num_bits < n) {
            if(inpos < inlength) {
                bit_buffer |= indata[inpos++] << num_bits;
                num_bits += 8;
            } else {
                return true; // Not enough data
            }
        }
    }
    return false;
}

void BitStreamDecoder::AlignToByte() {
    if(num_bits & 0x7) {
        num_unused += num_bits & 0x7;
        DropBits(num_bits & 0x7);
    }
}

uint32_t BitStreamDecoder::LoadFull() {
    while(num_bits <= 24) {
        if(!(bit_buffer < 1u << num_bits)) {
            return 2; // Bit buffer corrupt
        }
        if(inpos < inlength) {
            bit_buffer |= indata[inpos++] << num_bits;
            num_bits += 8;
        } else {
            return 1; // Not enough data
        }
    }
    return 0;
}

void BitStreamDecoder::DropBits(uint32_t n) {
    if(n > 32) {
        return;
    }
    if(num_bits < n) {
        return;
    }
    bit_buffer >>= n;
    num_bits -= n;
}

int32_t BitStreamDecoder::GetBits(uint32_t n) {
    if(n >= 32) {
        return -5; // Invalid input
    }
    if(num_bits < n) {
        uint32_t st;
        st = LoadFull();
        if(num_bits < n && st) {
            return -1;
        }
    }
    uint32_t rbits = bit_buffer & (~(0xffffffffu << n));
    bit_buffer >>= n;
    num_bits -= n;
    return (int32_t)rbits;
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
// zlib decode - Sean Barrett, originally found in SOIL
// modifications for C++ and state keeping - Meisaka Yukara

static const void_er BIT_MORE = void_er(1, "More data needed");
static const void_er BITERROR = void_er(2, "Bit buffer corrupt");

void_er Huffman::Build(const uint8_t *sizelist, uint32_t num) {
    uint32_t i;
    uint16_t code;
    uint32_t next_code[16], sizes[17];

    // DEFLATE spec for generating codes
    n_memarrayset(sizes, 0u, sizeof(sizes));
    n_memarrayset(this->fast, (uint16_t)0xffffu, sizeof(this->fast));
    for(i = 0; i < num; ++i) {
        if(!(sizelist[i] <= 16)) return void_er(-5, "Bad sizelist", __LINE__);
        ++sizes[sizelist[i]];
    }
    sizes[0] = 0;
    for(i = 1; i < 16; ++i) {
        if(!(sizes[i] <= (1u << i))) {
            return void_er(-5, "Bad sizes", __LINE__);
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
                return void_er(-5, "Bad codelengths", __LINE__);
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

uint16_t Inflate::HuffmanDecode(const Huffman& z) {
    uint32_t codesize;
    uint32_t b, k;
    bool st = instream.Require(16);
    if(st) {
        return 0xffffu;
    }
    b = z.fast[instream.bit_buffer & 0x1FF];
    if(b < 0xffff) {
        if(!(b < 288)) {
            error_state = void_er(-4, "Index out of range", __LINE__);
            return 0xfffeu;
        }
        codesize = z.size[b];
        instream.DropBits(codesize);
        return z.value[b];
    }

    // not resolved by fast table, so compute it
    k = BitReverse(instream.bit_buffer, 16);

    // validate
    for(codesize = 10; k >= z.maxcode[codesize] && codesize < 16; ++codesize);
    if(codesize == 16) {
        error_state = void_er(-3, "Code invalid", __LINE__);
        return 0xfffeu;
    }

    b = (k >> (16 - codesize)) - z.firstcode[codesize] + z.firstsymbol[codesize];
    if(!(b < 288)) {
        error_state = void_er(-4, "Index out of range", __LINE__);
        return 0xfffeu;
    }
    if(!z.size[b] == codesize) {
        error_state = void_er(-3, "Size table invalid", __LINE__);
        return 0xfffeu;
    }
    instream.DropBits(codesize);
    return z.value[b];
}

void_er Inflate::UncompressedBlock() {

    uint32_t length = 0;
    if(readstate == InflateState::BLOCK_UNCOMPRESSED) {
        instream.AlignToByte();
        if(instream.Require(32)) {
            return BIT_MORE;
        }
        uint32_t invlength;
        int32_t rbits;
        rbits = instream.GetBits(8);
        if(rbits < 0) {
            return BITERROR;
        }
        length = rbits << 8;
        rbits = instream.GetBits(8);
        if(rbits < 0) {
            return BITERROR;
        }
        length |= rbits & 0xff;
        rbits = instream.GetBits(8);
        if(rbits < 0) {
            return BITERROR;
        }
        invlength = rbits << 8;
        rbits = instream.GetBits(8);
        if(rbits < 0) {
            return BITERROR;
        }
        invlength |= rbits & 0xff;
        if(length != ~invlength) {
            return void_er(-3, "Non-compressed block invalid", __LINE__);
        }
        readstate = InflateState::BLOCK_UNCOMPRESSED_DATA;
    }
    else {
        length = s_num_literal_codes;
    }
    if(readstate == InflateState::BLOCK_UNCOMPRESSED_DATA) {
        if(length && instream.inpos < instream.inlength) {
            if(length + instream.inpos >= instream.inlength) {
                outdata.append(instream.indata.data() + instream.inpos, instream.inlength - instream.inpos);
                length -= instream.inlength - instream.inpos;
                if(length) {
                    return BIT_MORE;
                }
            }
            else {
                outdata.append(instream.indata.data() + instream.inpos, length);
            }
        }
        else if(length) {
            return BIT_MORE;
        }
    }
    return void_er();
}

void_er Inflate::DynamicBlock() {
    static const uint8_t ALPHABET_LENGTHS[19] = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };

    uint8_t codelength_sizes[19];
    int32_t rbits;
    uint32_t n;

    if(readstate == InflateState::BLOCK_DYNAMIC) {
        bool rbool = instream.Require(14);
        if(rbool) return BIT_MORE;
        rbits = instream.GetBits(5);
        if(rbits < 0) return BITERROR;
        s_num_literal_codes  = (uint32_t)rbits + 257;
        rbits = instream.GetBits(5);
        if(rbits < 0) return BITERROR;
        s_num_distance_codes = (uint32_t)rbits + 1;
        rbits = instream.GetBits(4);
        if(rbits < 0) return BITERROR;
        s_num_codelen_codes  = (uint32_t)rbits + 4;
        readstate = InflateState::BLOCK_DYNAMIC_CODELEN;
    }

    if(readstate == InflateState::BLOCK_DYNAMIC_CODELEN) {
        bool rbool = instream.Require(s_num_codelen_codes * 3);
        if(rbool) return BIT_MORE;

        s_codelength = std::unique_ptr<Huffman>(new Huffman());

        n_memarrayset(codelength_sizes, (uint8_t)0, sizeof(codelength_sizes));
        for(uint32_t i = 0; i < s_num_codelen_codes; ++i) {
            rbits = instream.GetBits(3);
            if(rbits < 0) {
                return BITERROR;
            }
            codelength_sizes[ALPHABET_LENGTHS[i]] = (uint8_t)rbits;
        }
        void_er ret = s_codelength->Build(codelength_sizes, 19);
        ret.line = __LINE__;
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
            sym = HuffmanDecode(*s_codelength);
            if(sym == 0xffffu) {
                return BIT_MORE;
            }
            else if(sym == 0xfffeu) {
                return error_state;
            }
            readstate = InflateState::BLOCK_DYNAMIC_SYMEXT;
        } else {
            sym = s_symbol;
        }
        if( !(sym >= 0u && sym < 19u) ) return void_er(-2, "Invalid decode", __LINE__);
        if(sym < 16u) {
            s_lencodes->v[n++] = (uint8_t)sym;
        }
        else if(sym == 16) {
            rbits = instream.GetBits(2);
            if(rbits == -1) {
                s_symbol = sym;
                return BIT_MORE;
            }
            else if(rbits < 0) {
                return BITERROR;
            }
            lc = rbits + 3;
            n_memarrayset(s_lencodes->v, n, s_lencodes->v[n - 1], lc);
            n += lc;
        }
        else if(sym == 17) {
            rbits = instream.GetBits(3);
            if(rbits == -1) {
                s_symbol = sym;
                return BIT_MORE;
            }
            else if(rbits < 0) {
                return BITERROR;
            }
            lc = rbits + 3;
            n_memarrayset(s_lencodes->v, n, (uint8_t)0, lc);
            n += lc;
        }
        else {
            //assert(sym == 18);
            rbits = instream.GetBits(7);
            if(rbits == -1) {
                s_symbol = sym;
                return BIT_MORE;
            }
            else if(rbits < 0) {
                return BITERROR;
            }
            lc = rbits + 11;
            n_memarrayset(s_lencodes->v, n, (uint8_t)0, lc);
            n += lc;
        }
        readstate = InflateState::BLOCK_DYNAMIC_SYMREAD;
    }
    s_codelength.reset();
    if(n != s_num_literal_codes + s_num_distance_codes) {
        readstate = InflateState::BAD_STREAM;
        return void_er(-3, "Bad code lengths", __LINE__);
    }
    void_er ret = lengthcodes.Build(s_lencodes->v, s_num_literal_codes);
    ret.line = __LINE__;
    if(ret) {
        readstate = InflateState::BAD_STREAM;
        return ret;
    }
    ret = distancecodes.Build(s_lencodes->v + s_num_literal_codes, s_num_distance_codes);
    ret.line = __LINE__;
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

    while(true) {
        if(readstate == InflateState::BLOCK_DATA_SYMBOL) {
            symbol = s_symbol;
            readstate = InflateState::BLOCK_DATA;
        } else {
            symbol = HuffmanDecode(lengthcodes);
            if(symbol == 0xffffu) {
                return BIT_MORE;
            }
            else if(symbol == 0xfffeu) {
                return error_state;
            }
        }
        if(symbol < 256) {
            csym = (unsigned char)symbol;
            outdata.append(&csym, 1);
        } else if(symbol == 256) {
            return void_er();
        } else {
            bool rbool = instream.Require(18+16);
            if(rbool) {
                s_symbol = symbol;
                readstate = InflateState::BLOCK_DATA_SYMBOL;
                return BIT_MORE;
            }
            symbol -= 257;
            length = length_base[symbol];
            if(length_extra[symbol]) {
                int32_t rbits = instream.GetBits(length_extra[symbol]);
                if(rbits < 0) {
                    return BITERROR;
                }
                length += rbits;
            }
            if(ret) {
                s_symbol = symbol;
                readstate = InflateState::BLOCK_DATA_SYMBOL;
                return ret;
            }
            symbol = HuffmanDecode(distancecodes);
            if(symbol == 0xffffu) {
                s_symbol = symbol;
                readstate = InflateState::BLOCK_DATA_SYMBOL;
                return BIT_MORE;
            }
            else if(symbol == 0xfffeu) {
                return error_state;
            }
            distance = dist_base[symbol];
            if(dist_extra[symbol]) {
                int32_t rbits = instream.GetBits(dist_extra[symbol]);
                if(rbits < 0) {
                    return BITERROR;
                }
                distance += rbits;
            }
            if(ret) {
                s_symbol = symbol;
                readstate = InflateState::BLOCK_DATA_SYMBOL;
                return ret;
            }
            outpos = outdata.length();
            if(outpos < distance) {
                return void_er(-2, "Invalid distance - output too short", __LINE__);
            }
            outpos -= distance;
            while(length--) {
                // this operation must be done one byte at a time
                // since we often end up reading bytes we append.
                // detecting end overlaps could possibly be used to improve performance
                const uint8_t * outptr = outdata.data() + outpos;
                outdata.append(outptr, 1);
                outpos++;
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
    if(instream.indata.length() == 0) {
        instream.indata.assign(data);
        instream.inlength = instream.indata.length();
    }
    else {
        instream.AppendData(data);
    }
    bool r = DecompressPass();
    instream.Flush();
    return r;
}

bool Inflate::DecompressData(const DataString && data) {
    if(instream.indata.length() == 0) {
        instream.indata.assign(std::move(data));
        instream.inlength = instream.indata.length();
    }
    else {
        instream.AppendData(data);
    }
    bool r = DecompressPass();
    instream.Flush();
    return r;
}

bool Inflate::DecompressPass() {
    uint8_t final_flag, type;
    if(readstate == InflateState::ZLIB_HEADER) {
        bool r = instream.Require(16);
        if(r) return false;
        instream.GetBits(8);
        instream.GetBits(8);
        checksum.Init();
        readstate = InflateState::BLOCK_HEADER;
    }

    error_state.error_code = 0;
    final_flag = readstate == InflateState::END_OF_STREAM ? 1 : 0;

    while(!final_flag && !error_state) {
        if(readstate == InflateState::BLOCK_HEADER) {
            bool r = instream.Require(3);
            if(r) return false;
            int32_t rbits = instream.GetBits(1);
            if(rbits < 0) {
                error_state = BITERROR;
                return true;
            }
            final_flag = rbits;
            s_final = final_flag;
            rbits = instream.GetBits(2);
            if(rbits < 0) {
                error_state = BITERROR;
                return true;
            }
            type = rbits;
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
        case InflateState::BLOCK_UNCOMPRESSED_DATA:
            error_state = UncompressedBlock();
            if(error_state.error_code == 1) return false;
            if(error_state) return true;
            readstate = InflateState::BLOCK_HEADER;
            break;
        case InflateState::BLOCK_DYNAMIC:
        case InflateState::BLOCK_DYNAMIC_CODELEN:
        case InflateState::BLOCK_DYNAMIC_SYMREAD:
        case InflateState::BLOCK_DYNAMIC_SYMEXT:
            error_state = DynamicBlock();
            if(error_state.error_code == 1) {
                return false;
            }
            if(error_state) {
                return true;
            }
            readstate = InflateState::BLOCK_DATA;
            // fall through
        case InflateState::BLOCK_DATA:
        case InflateState::BLOCK_DATA_SYMBOL:
            error_state = HuffmanBlock();
            if(error_state.error_code == 1) {
                return false;
            }
            if(error_state) {
                return true;
            }
            readstate = InflateState::BLOCK_HEADER;
            if(final_flag) {
                readstate = InflateState::END_OF_STREAM;
            }
            break;
        case InflateState::BAD_STREAM:
            error_state = void_er(-3, "Bad stream");
            return true;
            break;
        default:
            return false;
            break;
        }
    }
    instream.AlignToByte();
    bool r = instream.Require(32);
    if(r) return false;
    uint32_t checkval;
    checkval = instream.GetBits(8);
    checkval <<= 24;
    checkval |= instream.GetBits(8) << 16;
    checkval |= instream.GetBits(8) << 8;
    checkval |= instream.GetBits(8);
    checksum.Update(outdata.data(), outdata.length());
    checksum.Last();
    if(checksum.ldata != checkval) {
        error_state = void_er(-3, "Checksum failed", __LINE__);
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
