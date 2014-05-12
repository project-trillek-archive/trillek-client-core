
#ifndef UTILTYPE_HPP_INCLUDED
#define UTILTYPE_HPP_INCLUDED

#include <string>
#include <istream>
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

typedef ErrorReturn<void> void_er;

uint16_t BitReverse16(uint16_t n);
uint32_t BitReverse32(uint32_t n);
uint32_t BitReverse(uint32_t v, int bits);

std::istream &operator>>(std::istream &f, FourCC &o);

} // util
} // trillek
#endif
