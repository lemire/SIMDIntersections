/*
 * This is a simple implementation of the Skipping data structure and algorithms described in
 *
 * Sanders and Transier, Intersection in Integer Inverted Indices, 2007.
 *
 * As suggested in their conclusion, we leave the higher-level structure uncompressed.
 *
 * Sanders and Transier's proposal is similar in spirit to the skipping structure proposed in
 *
 * Moffat, A., Zobel, J.: Self-indexing inverted files for fast text retrieval.
 * ACM Transactions on Information Systems 14 (1996).
 *
 *
 *      Author: Daniel Lemire
 */

#ifndef SKIPPING_H_
#define SKIPPING_H_

#include "common.h"

class Skipping {
public:
    Skipping(uint32_t BS) : BlockSizeLog(BS),
        mainbuffer(), highbuffer(), Length(0) {
        assert((BlockSizeLog > 0) && (BlockSizeLog < 32));
    }

    ~Skipping() {}

    Skipping(uint32_t BS, uint32_t * data, uint32_t length) :
        BlockSizeLog(BS),
        mainbuffer(), highbuffer(), Length(0) {
        assert((BlockSizeLog > 0) && (BlockSizeLog < 32));
        load(data, length);// cheap constructor
    }

    void load(uint32_t * data, uint32_t length);

    uint32_t intersect(Skipping & otherlarger, uint32_t * out) {
        // we assume that "this" is the smallest of the two
        if (otherlarger.Length < Length)
            return otherlarger.intersect(*this, out);
        if (Length == 0)
            return 0;// special silly case
        uint32_t intersectsize = 0;

        const uint8_t * inbyte =
                reinterpret_cast<const uint8_t *> (mainbuffer.data());
        const uint8_t * const endbyte =
                reinterpret_cast<const uint8_t *> (mainbuffer.data()
                        + mainbuffer.size());
        const uint8_t * largemainpointer = otherlarger.mainbuffer.data();
        uint32_t largemainval = 0;
        largemainpointer = decode(largemainpointer, largemainval);
        uint32_t x = 0;
        uint32_t val = 0;// where I put decoded values

        while (endbyte > inbyte) {

            inbyte = decode(inbyte, val);

            if (otherlarger.highbuffer[x >> BlockSizeLog].first > val) {
                do {
                    x += 1 << BlockSizeLog;
                    if (x >= otherlarger.Length)
                        goto END_OF_MAIN;
                } while (otherlarger.highbuffer[x >> BlockSizeLog].first > val);
                largemainpointer
                        = otherlarger.mainbuffer.data() + otherlarger.highbuffer[x >> BlockSizeLog].second;
                largemainpointer = decode(largemainpointer, largemainval);
            }
            while (largemainval < val) {
                ++x;
                if (x >= otherlarger.Length)
                    goto END_OF_MAIN;
                largemainpointer = decode(largemainpointer, largemainval);
            }
            if (largemainval == val) {
                out[intersectsize++] = val;
            }
        }
        END_OF_MAIN: return x;
    }

    uint32_t BlockSizeLog;
    vector<uint8_t> mainbuffer;
    typedef vector<pair<uint32_t, uint32_t>> higharray;
    higharray highbuffer;
    uint32_t Length;

private:
    template<uint32_t i>
    uint8_t extract7bits(const uint32_t val) {
        return static_cast<uint8_t> ((val >> (7 * i)) & ((1U << 7) - 1));
    }

    template<uint32_t i>
    uint8_t extract7bitsmaskless(const uint32_t val) {
        return static_cast<uint8_t> ((val >> (7 * i)));
    }

    static inline const uint8_t * decode(const uint8_t * buffer, uint32_t& prev) {
        for (uint32_t v = 0, shift = 0;; shift += 7) {
            uint8_t c = *buffer++;
            v += ((c & 127) << shift);
            if ((c & 128)) {
                prev += v;
                return buffer;
            }
        }
    }
};

void Skipping::load(uint32_t * data, uint32_t length) {
    assert(length < (numeric_limits<size_t>::max() / 5));// check for overflow
    Length = length;
    uint32_t BlockNumber = (Length + (1<<BlockSizeLog) - 1) / (1<<BlockSizeLog);// count full blocks
    highbuffer.resize(BlockNumber);
    mainbuffer.resize(5 * BlockNumber);
    uint8_t * bout = mainbuffer.data();
    uint8_t * const boutinit = bout;
    for (uint32_t k = 0; k < BlockNumber; ++k) {
        highbuffer[k] = make_pair(data[k << BlockSizeLog],
                static_cast<uint32_t> (bout - boutinit));
        const uint32_t howmany = (((k + 1)  << BlockSizeLog) > Length) ?
                Length - (k << BlockSizeLog)
                : 1 << BlockSizeLog;
        for (uint32_t x = 0; x < howmany; ++x) {
            const uint32_t val = data[x + (k << BlockSizeLog)];
            if (val < (1U << 7)) {
                *bout = static_cast<uint8_t> (val | (1U << 7));
                ++bout;
            } else if (val < (1U << 14)) {
                *bout = extract7bits<0> (val);
                ++bout;
                *bout = extract7bitsmaskless<1> (val) | (1U << 7);
                ++bout;
            } else if (val < (1U << 21)) {
                *bout = extract7bits<0> (val);
                ++bout;
                *bout = extract7bits<1> (val);
                ++bout;
                *bout = extract7bitsmaskless<2> (val) | (1U << 7);
                ++bout;
            } else if (val < (1U << 28)) {
                *bout = extract7bits<0> (val);
                ++bout;
                *bout = extract7bits<1> (val);
                ++bout;
                *bout = extract7bits<2> (val);
                ++bout;
                *bout = extract7bitsmaskless<3> (val) | (1U << 7);
                ++bout;
            } else {
                *bout = extract7bits<0> (val);
                ++bout;
                *bout = extract7bits<1> (val);
                ++bout;
                *bout = extract7bits<2> (val);
                ++bout;
                *bout = extract7bits<3> (val);
                ++bout;
                *bout = extract7bitsmaskless<4> (val) | (1U << 7);
                ++bout;
            }
        }
    }
    mainbuffer.resize(static_cast<uint32_t> (bout - boutinit));
    vector<uint8_t> ().swap(mainbuffer);
}

#endif /* SKIPPING_H_ */
