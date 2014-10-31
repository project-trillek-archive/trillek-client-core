
#ifndef UTILTYPE_HPP_INCLUDED
#define UTILTYPE_HPP_INCLUDED

#include <string>
#include <istream>
#include <ostream>
#include <stdint.h>
#include <memory>

namespace trillek {
namespace util {

typedef std::basic_string<
    unsigned char,
    std::char_traits<unsigned char>,
    std::allocator<unsigned char>
> DataString;

struct FourCC {
    union {
        char cdata[4];
        uint32_t ldata;
    };
    FourCC();
    FourCC(char a, char b, char c, char d);
    FourCC(const char * a);
    FourCC(std::istream &f);
    bool operator==(const FourCC r) const {
        return ldata == r.ldata;
    }
    bool operator!=(const FourCC r) const {
        return ldata != r.ldata;
    }
};

template<class> class ErrorReturn;

template<>
class ErrorReturn<void> {
public:
    int error_code;
    //std::string error_text;
    const char * p_error_text;
    int32_t line;

    std::string Text() {
        if(p_error_text != nullptr) {
            return std::string(p_error_text);
        }
        return std::string();
    }
    ErrorReturn() :
        error_code(0), p_error_text(nullptr), line(-1) {}
    ~ErrorReturn() {}

    ErrorReturn(int code) :
        error_code(code), p_error_text(nullptr), line(-1) {}

    ErrorReturn(int code, const char * msg) :
        error_code(code), p_error_text(msg), line(-1) {}

    ErrorReturn(int code, const char * msg, int32_t l) :
        error_code(code), p_error_text(msg), line(l) {}

//    ErrorReturn(int code, const std::string& msg) :
//        error_code(code), error_text(msg) {}

//    ErrorReturn(int code, std::string&& msg) :
//        error_code(code), error_text(msg) {}

    ErrorReturn(const ErrorReturn &) = default;
    template<typename T>
    ErrorReturn(const ErrorReturn<T> & rv) {
        error_code = rv.error_code;
        //error_text = std::string(rv.error_text);
        p_error_text = rv.error_text;
        line = rv.line;
    }

    ErrorReturn& operator=(const ErrorReturn & rv) {
        error_code = rv.error_code;
        //error_text = std::string(rv.error_text);
        p_error_text = rv.p_error_text;
        line = rv.line;
        return *this;
    }
    ErrorReturn& operator=(ErrorReturn && rv) {
        error_code = rv.error_code;
        //error_text = std::move(rv.error_text);
        p_error_text = rv.p_error_text;
        line = rv.line;
        return *this;
    }
    template<class T>
    ErrorReturn<T> value (T val) {
        return ErrorReturn<T>(val, *this);
    }

    template<typename T>
    T& operator=(ErrorReturn<T> & rv) {
        error_code = rv.error_code;
        //error_text = std::string(rv.error_text);
        p_error_text = rv.p_error_text;
        line = rv.line;
        return rv.value;
    }

    operator bool() const {
        return error_code != 0;
    }

    template<typename T>
    ErrorReturn(ErrorReturn<T> && rv) {
        error_code = rv.error_code;
        //error_text = std::move(rv.error_text);
        p_error_text = rv.p_error_text;
        line = rv.line;
    }
    template<typename T>
    T& operator=(ErrorReturn<T> && rv) {
        error_code = rv.error_code;
        //error_text = std::move(rv.error_text);
        p_error_text = rv.p_error_text;
        line = rv.line;
        return rv.value;
    }
};

template<class RT>
class ErrorReturn {
    friend class ErrorReturn<void>;
public:
    RT value;
    int error_code;
    int32_t line;
    //std::string error_text;
protected:
    const char * p_error_text;
public:
    std::string Text() {
        if(p_error_text != nullptr) {
            return std::string(p_error_text);
        }
        return std::string();
    }
    ErrorReturn(RT val) :
        value(val), error_code(0), p_error_text(nullptr), line(-1) {}
    ~ErrorReturn() {}

    ErrorReturn(RT val, int code) :
        value(val), error_code(code), p_error_text(nullptr), line(-1) {}

    ErrorReturn(RT val, int code, const char * msg) :
        value(val), error_code(code), p_error_text(msg), line(-1) {}

    ErrorReturn(RT val, int code, const char * msg, int32_t l) :
        value(val), error_code(code), p_error_text(msg), line(l) {}

//    ErrorReturn(RT val, int code, const std::string& msg) :
//        value(val), error_code(code), error_text(msg) {}

//    ErrorReturn(RT val, int code, std::string&& msg) :
//        value(val), error_code(code), error_text(msg) {}

    ErrorReturn(const ErrorReturn &) = default;
    ErrorReturn& operator=(const ErrorReturn &) = default;

    ErrorReturn(RT val, const ErrorReturn<void> &rv) : value(val) {
        error_code = rv.error_code;
        p_error_text = rv.p_error_text;
        line = rv.line;
    }
    ErrorReturn(RT val, ErrorReturn<void> &&rv) : value(val) {
        error_code = rv.error_code;
        p_error_text = std::move(rv.p_error_text);
        line = rv.line;
    }
    operator bool() const {
        return error_code != 0;
    }

    RT& operator*() {
        return value;
    }

    ErrorReturn(ErrorReturn && rv) {
        value = std::move(rv.value);
        error_code = rv.error_code;
        p_error_text = std::move(rv.p_error_text);
        line = rv.line;
    }
    ErrorReturn& operator=(ErrorReturn && rv) {
        value = std::move(rv.value);
        error_code = rv.error_code;
        p_error_text = std::move(rv.p_error_text);
        line = rv.line;
        return *this;
    }
};
typedef ErrorReturn<void> void_er;

class InputStream {
public:
    InputStream() {}
    virtual ~InputStream() {};
    InputStream(const InputStream &) = delete;
    InputStream& operator=(const InputStream &) = delete;
    InputStream(InputStream &&) {}
    virtual InputStream& operator=(InputStream &&) = 0;

    virtual bool End() = 0;
    virtual uint8_t Read() = 0;
    virtual DataString Read(uint32_t l) {
        DataString data;
        while(!End() && l > 0) {
            uint8_t c = Read();
            data.append(&c, 1);
            l--;
        }
        return std::move(data);
    }
};

class StdInputStream : public InputStream {
public:
    StdInputStream();
    StdInputStream(std::istream & i) : inputstream(i) {}
    virtual ~StdInputStream() {}
    virtual InputStream& operator=(InputStream &&) { return *this; }

    StdInputStream(StdInputStream && x) : inputstream(x.inputstream) {}

    virtual bool End() {
        return inputstream.eof();
    }
    virtual uint8_t Read() {
        return (uint8_t)inputstream.get();
    }
    virtual DataString Read(uint32_t l) {
        DataString data;
        uint8_t block[256];
        while(l >= 256) {
            inputstream.read((char*)block, 256);
            data.append(block, 256);
            l -= 256;
        }
        if(l > 0) {
            inputstream.read((char*)block, l);
            data.append(block, l);
        }
        return std::move(data);
    }

protected:
    std::istream & inputstream;
};

class InputFilter : public InputStream {
public:
    InputFilter(InputStream & f) : forward(f) {}
    virtual InputStream& operator=(InputStream &&) { return *this; }
    virtual bool End() {
        return forward.End();
    }
    virtual uint8_t Read() {
        return Filter(forward.Read());
    }
    virtual uint8_t Filter(uint8_t i) {
        return i;
    }
protected:
    InputStream & forward;
};

uint16_t BitReverse16(uint16_t n);
uint32_t BitReverse32(uint32_t n);
uint32_t BitReverse(uint32_t v, int bits);

InputStream& operator>>(InputStream & f, uint8_t & o);
InputStream& operator>>(InputStream & f, FourCC & o);

std::istream& operator>>(std::istream & f, FourCC & o);
std::ostream& operator<<(std::ostream & f, FourCC & o);

template<class T>
inline int PopCount(T);

template<>
inline int PopCount<uint64_t>(uint64_t value) {
    #if defined(__GNUG__)
        return __builtin_popcountll(value);
    #elif defined(_MSC_VER)
        return __popcnt(value);
    #endif
}

template<>
inline int PopCount<uint32_t>(uint32_t value) {
    #if defined(__GNUG__)
        return __builtin_popcountl(value);
    #elif defined(_MSC_VER)
        return __popcnt(value);
    #endif
}

template<class T>
inline uint32_t Ctz(T);

template<>
inline uint32_t Ctz<uint32_t>(uint32_t value) {
#if defined(__GNUG__)
        return static_cast<uint32_t>(__builtin_ctzl(value));
#elif defined(_MSC_VER)
        unsigned long ret;
        _BitScanForward(&ret, value);
        return ret;
#endif
}

#if defined(__GNUG__)
template<>
inline uint32_t Ctz<uint64_t>(uint64_t value) {
        return static_cast<uint32_t>(__builtin_ctzll(value));
}
#endif

template<class T>
inline unsigned int Log2Bin();

template<>
inline unsigned int Log2Bin<uint32_t>() { return 5; };

template<>
inline unsigned int Log2Bin<uint64_t>() { return 6; };

template<class T>
struct is_shared_ptr : std::false_type {};

template<class T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

} // util
} // trillek
#endif
