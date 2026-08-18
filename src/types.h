#pragma once
#include <cstdint>

typedef uint64_t U64;
typedef uint8_t U8;

enum Side { BUY, SELL };

inline int getLSB(U64 word) {
    unsigned long index;
    _BitScanForward64(&index, word);
    return (int)index;
}

inline int getMSB(U64 word) {
    unsigned long index;
    _BitScanForward64(&index, word);
    return (int)index;
}