
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

InflateState::InflateState() {
    outdata = DataString();
    outpos  = 0;

    errored = false;

    readstate = InflateStateNumber::HEADER;
}

void_er BitStreamDecoder::AppendData(const DataString & in)
{
    indata.append(in);
    return void_er();
}

void_er BitStreamDecoder::FetchByte() {
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
void_er BitStreamDecoder::FetchFull() {
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

ErrorReturn<uint32_t> BitStreamDecoder::GetBits(int n) {
    if(n > 32) {
        return ErrorReturn<uint32_t>(0, -5, "Invalid input");
    }
    if(num_bits < n) {
        void_er st;
        st = FetchFull();
        if(num_bits < n && st) {
            return st.value<uint32_t>(0u);
        }
    }
    uint32_t rbits = bit_buffer & ((1u << n) - 1);
    bit_buffer >>= n;
    num_bits -= n;
    return ErrorReturn<uint32_t>(rbits);
}

Inflate::Inflate() {
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
    state.instream.AppendData(data);
    uint32_t tmp;
    void_er status;

    tmp = status = state.instream.GetBits(5);
    if(status) {
        state.errored = true;
        return false;
    }
    state.error_state = status;
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
