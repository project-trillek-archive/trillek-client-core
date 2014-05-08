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
            FourCC() {
                ldata = 0;
            }
            FourCC(char a, char b, char c, char d) {
                cdata[0] = a;
                cdata[1] = b;
                cdata[2] = c;
                cdata[3] = d;
            }
            FourCC(const char * a) {
                cdata[0] = a[0];
                cdata[1] = a[1];
                cdata[2] = a[2];
                cdata[3] = a[3];
            }
            FourCC(std::istream &f) {
                for(int i = 0; i < 4 && !f.eof(); i++) {
                    cdata[i] = f.get();
                }
            }
            bool operator==(const FourCC r) {
                return ldata == r.ldata;
            }
            bool operator!=(const FourCC r) {
                return ldata != r.ldata;
            }
        };
        std::istream &operator>>(std::istream &f, FourCC &o) {
            for(int i = 0; i < 4 && !f.eof(); i++) {
                o.cdata[i] = f.get();
            }
            return f;
        }
    }
}
#endif
