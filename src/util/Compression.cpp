
#include "util/UtilType.hpp"
#include "util/Compression.hpp"

namespace trillek {
namespace util {
namespace algorithm {

InflateState::InflateState() {
    indata = std::string();
    inpos  = 0;

    outdata = std::string();
    outpos  = 0;

    num_bits = 0;
    bit_buffer = 0;

    errored = false;

    readstate = InflateStateNumber::HEADER;
}

ErrorReturn<void> InflateState::FetchByte() {
    if(num_bits <= 24) {
        if(inpos < indata.length()) {
            bit_buffer = (bit_buffer << 8) | indata[inpos++];
            num_bits += 8;
            return ErrorReturn<void>(0);
        } else {
            return ErrorReturn<void>(-1, "Not enough data");
        }
    } else {
        return ErrorReturn<void>(0);
    }
}

ErrorReturn<uint32_t> InflateState::GetBits(int n) {
    return ErrorReturn<uint32_t>(0);
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

bool Inflate::DecompressData(std::string data) {
    state.indata += data;
    uint32_t tmp;
    ErrorReturn<void> status;

    tmp = status = state.GetBits(5);
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

std::string Inflate::DecompressGetOutput() {
    return std::string();
}

} // algorithm
} // util
} // trillek
