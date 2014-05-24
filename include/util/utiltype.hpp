
#ifndef UTILTYPE_HPP_INCLUDED
#define UTILTYPE_HPP_INCLUDED

#include <string>
#include <istream>
#include <ostream>
#include <stdint.h>

namespace trillek {
namespace util {

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
    std::string error_text;

    ErrorReturn() :
        error_code(0), error_text() {}
    ~ErrorReturn() {}

    ErrorReturn(int code) :
        error_code(code), error_text() {}

    ErrorReturn(int code, const std::string& msg) :
        error_code(code), error_text(msg) {}

    ErrorReturn(int code, std::string&& msg) :
        error_code(code), error_text(msg) {}

    ErrorReturn(const ErrorReturn &) = default;
    template<typename T>
    ErrorReturn(const ErrorReturn<T> & rv) {
        error_code = rv.error_code;
        error_text = std::string(rv.error_text);
    }

    ErrorReturn& operator=(const ErrorReturn & rv) {
        error_code = rv.error_code;
        error_text = std::string(rv.error_text);
        return *this;
    }
    ErrorReturn& operator=(ErrorReturn && rv) {
        error_code = rv.error_code;
        error_text = std::move(rv.error_text);
        return *this;
    }
    template<class T>
    ErrorReturn<T> value (T val) {
        return ErrorReturn<T>(val, *this);
    }

    template<typename T>
    T& operator=(ErrorReturn<T> & rv) {
        error_code = rv.error_code;
        error_text = std::string(rv.error_text);
        return rv.value;
    }

    operator bool() {
        return error_code != 0;
    }

    template<typename T>
    ErrorReturn(ErrorReturn<T> && rv) {
        error_code = rv.error_code;
        error_text = std::move(rv.error_text);
    }
    template<typename T>
    T& operator=(ErrorReturn<T> && rv) {
        error_code = rv.error_code;
        error_text = std::move(rv.error_text);
        return rv.value;
    }
};

template<class RT>
class ErrorReturn {
public:
    RT value;
    int error_code;
    std::string error_text;

    ErrorReturn(RT val) :
        value(val), error_code(0), error_text() {}
    ~ErrorReturn() {}

    ErrorReturn(RT val, int code) :
        value(val), error_code(code), error_text() {}

    ErrorReturn(RT val, int code, const std::string& msg) :
        value(val), error_code(code), error_text(msg) {}

    ErrorReturn(RT val, int code, std::string&& msg) :
        value(val), error_code(code), error_text(msg) {}

    ErrorReturn(const ErrorReturn &) = default;
    ErrorReturn& operator=(const ErrorReturn &) = default;

    ErrorReturn(RT val, const ErrorReturn<void> &rv) : value(val) {
        error_code = rv.error_code;
        error_text = rv.error_text;
    }
    ErrorReturn(RT val, ErrorReturn<void> &&rv) : value(val) {
        error_code = rv.error_code;
        error_text = std::move(rv.error_text);
    }
    operator bool() {
        return error_code != 0;
    }

    RT& operator*() {
        return value;
    }

    ErrorReturn(ErrorReturn && rv) {
        value = std::move(rv.value);
        error_code = rv.error_code;
        error_text = std::move(rv.error_text);
    }
    ErrorReturn& operator=(ErrorReturn && rv) {
        value = std::move(rv.value);
        error_code = rv.error_code;
        error_text = std::move(rv.error_text);
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

} // util
} // trillek
#endif
