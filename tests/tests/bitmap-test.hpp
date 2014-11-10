#ifndef BITMAPTEST_H_INCLUDED
#define BITMAPTEST_H_INCLUDED

#include <stdexcept>
#include "bitmap.hpp"

namespace trillek {

class BitMapTest : public ::testing::Test {
    typedef std::default_random_engine::result_type rand;
public:

    template<class T>
    void Populate(BitMap<T>& a, BitMap<T>& b) {
        auto offset = next(std::min(a.size(),b.size()));
        auto offsetA = next(offset);
        auto offsetB = next(offset);
        refA = std::vector<bool>(a.size(), a.DefaultValue());
        refB = std::vector<bool>(b.size(), b.DefaultValue());
        auto minsize = std::min(a.size(),b.size());
        for(auto i = 0 ; i < minsize >> 1; ++i) {
            auto alea = next(minsize - offset) + offset;
            a[alea] = ! a.DefaultValue();
            b[alea] = ! b.DefaultValue();
            refA[alea] = ! a.DefaultValue();
            refB[alea] = ! b.DefaultValue();
        }

        for(auto i = 0 ; i < a.size() >> 1; ++i) {
            auto alea = next(a.size() - offsetA) + offsetA;
            a[alea] = ! a.DefaultValue();
            refA[alea] = ! a.DefaultValue();
        }

        for(auto i = 0 ; i < b.size() >> 1; ++i) {
            auto alea = next(b.size() - offsetB) + offsetB;
            b[alea] = ! b.DefaultValue();
            refB[alea] = ! b.DefaultValue();
        }
    }

    template<class T>
    bool Compare(BitMap<T>& a, BitMap<T>& b, const BitMap<T>& result, const std::function<bool(bool,bool)>& operation) {
        operate(a, b, operation);
        if (result.size() != refResult.size()) {
            return false;
        }
        for (auto i = 0; i < result.size(); ++i) {
            if (result.at(i) != refResult.at(i)) {
                return false;
            }
        }
        return true;
    }

    rand next(uint32_t max) {
        if(max) {
            return (random() / (random.max() / max));
        }
        return 0;
    }

    std::vector<bool> refA, refB, refResult;

private:
    template<class T>
    void operate(BitMap<T>& a, BitMap<T>& b, const std::function<bool(bool,bool)>& operation) {
        auto maxsize = std::max(refA.size(),refB.size());
        refA.resize(maxsize, a.DefaultValue());
        refB.resize(maxsize, b.DefaultValue());
        refResult = std::vector<bool>(maxsize, operation(a.DefaultValue(), b.DefaultValue()));
        for (auto i = 0; i < maxsize; ++i) {
            refResult[i] = operation(refA.at(i),refB.at(i));
        }
    }

    unsigned long int seed = (unsigned long int) std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine random{seed};
};

TEST_F(BitMapTest, BitMapBasic) {
    BitMap<uint32_t> bit_array((size_t) 550); //550 bits
    ASSERT_FALSE(bit_array[200]) << "Default value is not false";
    bit_array[200] = true;
    ASSERT_TRUE(bit_array[200]) << "Failed to write in array";
    bit_array[200] = false;
    ASSERT_FALSE(bit_array[200]) << "Failed to rewrite in array";
}

TEST_F(BitMapTest, BitMapExtend) {
    BitMap<uint32_t> bit_array((size_t) 550); //550 bits
    EXPECT_FALSE(bit_array[600]) << "Default value is not false";
    EXPECT_NO_THROW(bit_array[600] = true) << "Failed to extend array";
    EXPECT_TRUE(bit_array[600]) << "Failed to write in extended array";
}

TEST_F(BitMapTest, BitMapDefaultValue) {
    BitMap<uint32_t> bit_array((size_t) 550,true); //550 bits
    ASSERT_TRUE(bit_array[203]) << "Default value is not true";
    bit_array[203] = false;
    ASSERT_FALSE(bit_array[203]) << "Failed to write in array";
}

TEST_F(BitMapTest, BitMapAND) {
    for(auto i = 0; i < 5; ++i) {
        BitMap<uint32_t> a((size_t) next(2500));
        BitMap<uint32_t> b((size_t) next(2500));
        Populate(a, b);
        EXPECT_TRUE(Compare(a,b,a & b,[](bool a,bool b) { return a && b; }));
        for (int i = 0; i < a.size(); ++i) {
            ASSERT_EQ(refA[i], (bool) a[i]) << "error at " << i;
        }
        for (int i = 0; i < b.size(); ++i) {
            ASSERT_EQ(refB[i], (bool) b[i]) << "error at " << i;
        }
    }
}

TEST_F(BitMapTest, BitMapOR) {
    for(auto i = 0; i < 5; ++i) {
        BitMap<uint32_t> a((size_t) next(2500));
        BitMap<uint32_t> b((size_t) next(2500));
        Populate(a, b);
        EXPECT_TRUE(Compare(a,b,a | b,[](bool a,bool b) { return a || b; }));
        for (int i = 0; i < a.size(); ++i) {
            ASSERT_EQ(refA[i], (bool) a[i]) << "error at " << i;
        }
        for (int i = 0; i < b.size(); ++i) {
            ASSERT_EQ(refB[i], (bool) b[i]) << "error at " << i;
        }
    }
}

TEST_F(BitMapTest, BitMapXOR) {
    for(auto i = 0; i < 5; ++i) {
        BitMap<uint32_t> a((size_t) next(2500));
        BitMap<uint32_t> b((size_t) next(2500));
        Populate(a, b);
        EXPECT_TRUE(Compare(a,b,a ^ b,[](bool lhs,bool rhs) { return !( lhs && rhs ) && ( lhs || rhs ); }));
        for (int i = 0; i < a.size(); ++i) {
            ASSERT_EQ(refA[i], (bool) a[i]) << "error at " << i;
        }
        for (int i = 0; i < b.size(); ++i) {
            ASSERT_EQ(refB[i], (bool) b[i]) << "error at " << i;
        }
    }
}

TEST_F(BitMapTest, BitMapANDCompound) {
    for(auto i = 0; i < 5; ++i) {
        BitMap<uint32_t> a((size_t) next(2500));
        BitMap<uint32_t> b((size_t) next(2500));
        Populate(a, b);
        auto backup = a;
        a &= b;
        EXPECT_TRUE(Compare(backup,b,a,[](bool a,bool b) { return a && b; }));
        for (int i = 0; i < b.size(); ++i) {
            ASSERT_EQ(refB[i], (bool) b[i]) << "error at " << i;
        }
    }
}

TEST_F(BitMapTest, BitMapORCompound) {
    for(auto i = 0; i < 5; ++i) {
        BitMap<uint32_t> a((size_t) next(2500));
        BitMap<uint32_t> b((size_t) next(2500));
        Populate(a, b);
        auto backup = a;
        a |= b;
        EXPECT_TRUE(Compare(backup,b,a,[](bool a,bool b) { return a || b; }));
        for (int i = 0; i < b.size(); ++i) {
            ASSERT_EQ(refB[i], (bool) b[i]) << "error at " << i;
        }
    }
}

TEST_F(BitMapTest, BitMapXORCompound) {
    for(auto i = 0; i < 5; ++i) {
        BitMap<uint32_t> a((size_t) next(2500));
        BitMap<uint32_t> b((size_t) next(2500));
        Populate(a, b);
        auto backup = a;
        a ^= b;
        EXPECT_TRUE(Compare(backup,b,a,[](bool lhs,bool rhs) { return !( lhs && rhs ) && ( lhs || rhs ); }));
        for (int i = 0; i < b.size(); ++i) {
            ASSERT_EQ(refB[i], (bool) b[i]) << "error at " << i;
        }
    }
}

TEST_F(BitMapTest, BitMapNOT) {
    BitMap<uint32_t> b((size_t) next(2500));
    BitMap<uint32_t> a((size_t) next(2500));
    Populate(a,b);
    b = ~a;
    for (int i = 0; i < a.size(); ++i) {
        ASSERT_NE((bool) b[i], (bool) a[i]) << "error at " << i << " size is " << b.size();
        ASSERT_EQ(refA[i], (bool) a[i]) << "error at " << i;
    }
}

TEST_F(BitMapTest, BitMapCountTrue) {
    BitMap<uint32_t> bit_array((size_t) 573); //573 bits
    auto count = bit_array.countTrue();
    ASSERT_EQ(0, count) << "Empty array countTrue() should return 0";
    (bit_array)[0] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(1, count) << "countTrue() should return 1";
    (bit_array)[1] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(2, count) << "countTrue() should return 2";
    (bit_array)[15] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(3, count) << "countTrue() should return 3";
    (bit_array)[16] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(4, count) << "countTrue() should return 4";
    (bit_array)[100] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(5, count) << "countTrue() should return 5";
    (bit_array)[300] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(6, count) << "countTrue() should return 6";
    (bit_array)[569] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(7, count) << "countTrue() should return 7";
    (bit_array)[570] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(8, count) << "countTrue() should return 8";
    (bit_array)[571] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(9, count) << "countTrue() should return 9";
    (bit_array)[572] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(10, count) << "countTrue() should return 10";
}

TEST_F(BitMapTest, BitMapCountTrue2) {
    BitMap<uint32_t> bit_array((size_t) 573); //573 bits
    auto count = bit_array.countTrue();
    (bit_array)[100] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(1, count) << "countTrue() should return 1";
    (bit_array)[300] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(2, count) << "countTrue() should return 2";
    (bit_array)[569] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(3, count) << "countTrue() should return 3";
    (bit_array)[570] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(4, count) << "countTrue() should return 4";
    (bit_array)[571] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(5, count) << "countTrue() should return 5";
    (bit_array)[572] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(6, count) << "countTrue() should return 6";
}

TEST_F(BitMapTest, BitMapCountTrueDefaultTrue) {
    BitMap<uint32_t> bit_array((size_t) 573, true); //573 bits
    auto count = bit_array.countTrue();
    ASSERT_EQ(573, count) << "Empty array countTrue()";
    count = bit_array.countTrue();
    (bit_array)[100] = false;
    count = bit_array.countTrue();
    ASSERT_EQ(572, count) << "countTrue()";
    (bit_array)[300] = false;
    count = bit_array.countTrue();
    ASSERT_EQ(571, count) << "countTrue()";
    (bit_array)[569] = false;
    count = bit_array.countTrue();
    ASSERT_EQ(570, count) << "countTrue()";
    (bit_array)[570] = false;
    count = bit_array.countTrue();
    ASSERT_EQ(569, count) << "countTrue()";
    (bit_array)[571] = false;
    count = bit_array.countTrue();
    ASSERT_EQ(568, count) << "countTrue()";
    (bit_array)[572] = false;
    count = bit_array.countTrue();
    ASSERT_EQ(567, count) << "countTrue()";
}
TEST_F(BitMapTest, BitMapEnumerator) {
    BitMap<uint32_t> bit_array((size_t) 576); //576 bits
    auto it1 = bit_array.enumerator(1000);
    ASSERT_LE(576, ++it1) << "it++ should return size when BitMap is empty";

    (bit_array)[0] = true;
    (bit_array)[1] = true;
    (bit_array)[15] = true;
    (bit_array)[63] = true;
    (bit_array)[64] = true;
    (bit_array)[65] = true;
    (bit_array)[300] = true;
    (bit_array)[574] = true;
    (bit_array)[575] = true;

    auto it = bit_array.enumerator(1000);
    ASSERT_EQ(0, *it) << "it++ should return 0";
    ASSERT_EQ(1, ++it) << "it++ should return 1";
    ASSERT_EQ(15, ++it) << "it++ should return 15";
    ASSERT_EQ(63, ++it) << "it++ should return 63";
    ASSERT_EQ(64, ++it) << "it++ should return 64";
    ASSERT_EQ(65, ++it) << "it++ should return 65";
    ASSERT_EQ(300, ++it) << "it++ should return 300";
    ASSERT_EQ(574, ++it) << "it++ should return 574";
    ASSERT_EQ(575, ++it) << "it++ should return 575";
    ASSERT_LE(576, ++it) << "it++ should return size";

    BitMap<uint32_t> bit_array2((size_t) 576); //576 bits
    (bit_array2)[1] = true;
    auto it2 = bit_array2.enumerator(1000);
    ASSERT_EQ(1, *it2) << "it++ should return 1";
}

#if defined(__GNUG__)
TEST_F(BitMapTest, BitMapEnumerator64) {
    BitMap<uint64_t> bit_array((size_t) 576); //576 bits
    auto it1 = bit_array.enumerator(1000);
    ASSERT_LE(576, ++it1) << "it++ should return size when BitMap is empty";

    (bit_array)[0] = true;
    (bit_array)[1] = true;
    (bit_array)[15] = true;
    (bit_array)[63] = true;
    (bit_array)[64] = true;
    (bit_array)[65] = true;
    (bit_array)[300] = true;
    (bit_array)[574] = true;
    (bit_array)[575] = true;

    auto it = bit_array.enumerator(1000);
    ASSERT_EQ(0, *it) << "it++ should return 0";
    ASSERT_EQ(1, ++it) << "it++ should return 1";
    ASSERT_EQ(15, ++it) << "it++ should return 15";
    ASSERT_EQ(63, ++it) << "it++ should return 63";
    ASSERT_EQ(64, ++it) << "it++ should return 64";
    ASSERT_EQ(65, ++it) << "it++ should return 65";
    ASSERT_EQ(300, ++it) << "it++ should return 300";
    ASSERT_EQ(574, ++it) << "it++ should return 574";
    ASSERT_EQ(575, ++it) << "it++ should return 575";
    ASSERT_LE(576, ++it) << "it++ should return size";

    BitMap<uint64_t> bit_array2((size_t) 576); //576 bits
    (bit_array2)[1] = true;
    auto it2 = bit_array2.enumerator(1000);
    ASSERT_EQ(1, *it2) << "it++ should return 1";
}
TEST_F(BitMapTest, BitMapCountTrue64) {
    BitMap<uint64_t> bit_array((size_t) 573); //573 bits
    auto count = bit_array.countTrue();
    ASSERT_EQ(0, count) << "Empty array countTrue() should return 0";
    (bit_array)[0] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(1, count) << "countTrue() should return 1";
    (bit_array)[1] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(2, count) << "countTrue() should return 2";
    (bit_array)[15] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(3, count) << "countTrue() should return 3";
    (bit_array)[16] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(4, count) << "countTrue() should return 4";
    (bit_array)[100] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(5, count) << "countTrue() should return 5";
    (bit_array)[300] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(6, count) << "countTrue() should return 6";
    (bit_array)[569] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(7, count) << "countTrue() should return 7";
    (bit_array)[570] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(8, count) << "countTrue() should return 8";
    (bit_array)[571] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(9, count) << "countTrue() should return 9";
    (bit_array)[572] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(10, count) << "countTrue() should return 10";
}

TEST_F(BitMapTest, BitMapCountTrue2_64) {
    BitMap<uint64_t> bit_array((size_t) 573); //573 bits
    auto count = bit_array.countTrue();
    (bit_array)[100] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(1, count) << "countTrue() should return 1";
    (bit_array)[300] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(2, count) << "countTrue() should return 2";
    (bit_array)[569] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(3, count) << "countTrue() should return 3";
    (bit_array)[570] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(4, count) << "countTrue() should return 4";
    (bit_array)[571] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(5, count) << "countTrue() should return 5";
    (bit_array)[572] = true;
    count = bit_array.countTrue();
    ASSERT_EQ(6, count) << "countTrue() should return 6";
}

TEST_F(BitMapTest, BitMapCountTrueDefaultTrue64) {
    BitMap<uint64_t> bit_array((size_t) 573, true); //573 bits
    auto count = bit_array.countTrue();
    ASSERT_EQ(573, count) << "Empty array countTrue()";
    count = bit_array.countTrue();
    (bit_array)[100] = false;
    count = bit_array.countTrue();
    ASSERT_EQ(572, count) << "countTrue()";
    (bit_array)[300] = false;
    count = bit_array.countTrue();
    ASSERT_EQ(571, count) << "countTrue()";
    (bit_array)[569] = false;
    count = bit_array.countTrue();
    ASSERT_EQ(570, count) << "countTrue()";
    (bit_array)[570] = false;
    count = bit_array.countTrue();
    ASSERT_EQ(569, count) << "countTrue()";
    (bit_array)[571] = false;
    count = bit_array.countTrue();
    ASSERT_EQ(568, count) << "countTrue()";
    (bit_array)[572] = false;
    count = bit_array.countTrue();
    ASSERT_EQ(567, count) << "countTrue()";
}
#endif
}

#endif // BITARRAYTEST_H_INCLUDED
