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
    /*Skipping(uint32_t BS) : BlockSizeLog(BS),
        mainbuffer(), highbuffer(), Length(0) {
        if((BlockSizeLog == 0) && (BlockSizeLog >= 32)) throw runtime_error("please use a reasonnable BlockSizeLog");
    }*/

    ~Skipping() {}

    Skipping(uint32_t BS, uint32_t * data, uint32_t length) :
        BlockSizeLog(BS),
        mainbuffer(), highbuffer(), Length(0) {
        if((BlockSizeLog == 0) && (BlockSizeLog >= 32)) throw runtime_error("please use a reasonnable BlockSizeLog");
        load(data, length);// cheap constructor
if(mainbuffer.size()==0) throw logic_error("ooooo");
    }

    uint32_t decompress(uint32_t * out) const {
        const uint8_t * bout = mainbuffer.data();
        uint32_t pos = 0;

        uint32_t val = 0;
        for(uint32_t k = 0; k < Length;++k) {
          bout = decode(bout,val);
          out[pos++] = val;
        }
        return pos;
    }


    uint32_t intersect(const Skipping & otherlarger, uint32_t * out) const {
        // we assume that "this" is the smallest of the two
        if (otherlarger.Length < Length)
            return otherlarger.intersect(*this, out);
        //cout<<"My length ="<<Length<<" other length = "<<otherlarger.Length<<endl;
        //cout<<"My mainlength ="<<mainbuffer.size()<<" other mainlength = "<<otherlarger.mainbuffer.size()<<endl;
        if (Length == 0)
            return 0;// special silly case
        assert(otherlarger.Length>=Length);
        assert(otherlarger.Length>0);
        uint32_t intersectsize = 0;

        vector<uint32_t> debug1(Length);
        decompress(debug1.data());
        vector<uint32_t> debug2(otherlarger.Length);
        otherlarger.decompress(debug2.data());

        const uint8_t * inbyte =
                reinterpret_cast<const uint8_t *> (mainbuffer.data());
        const uint8_t * const endbyte =
                reinterpret_cast<const uint8_t *> (mainbuffer.data()
                        + mainbuffer.size());
        const uint8_t * largemainpointer = otherlarger.mainbuffer.data();
        uint32_t largemainval = 0;
        assert(otherlarger.mainbuffer.size()>=otherlarger.Length);
        assert(largemainpointer < otherlarger.mainbuffer.data() + otherlarger.mainbuffer.size());
        largemainpointer = decode(largemainpointer, largemainval);
        uint32_t x = 0;
        if(debug2[x]!=largemainval) throw logic_error("no0");
      //  cout<<"(bef ev) I am at "<<x<<" with value "<<debug2[x]<<endl;

        uint32_t val = 0;// where I put decoded values
uint32_t y  = 0;
        while (endbyte > inbyte) {

            inbyte = decode(inbyte, val);
            if(val != debug1[y])  throw logic_error("no");
            ++y;
//cout<<"read from small="<<val<<endl;
         ///   cout<<"(beg main) I am at "<<x<<" with value "<<debug2[x]<<endl;

            if(debug2[x]!=largemainval) {
                cout<<"I am supposed to be at "<<x<<" the value there should be "<<debug2[x]<<endl;
                cout<<"instead I am getting "<<largemainval<<endl;
                throw logic_error("no11111");
            }

            if (otherlarger.highbuffer[x >> BlockSizeLog].first < val) {

            //    cout<<"(bef do) I am at "<<x<<" with value "<<debug2[x]<<endl;

                if (debug2[x] != largemainval) {
                    cout << "I am supposed to be at " << x
                            << " the value there should be " << debug2[x]
                            << endl;
                    cout << "instead I am getting " << largemainval << endl;
                    throw logic_error("no2222xxxxx");
                }
//                cout<<"(bef do) I am at "<<x<<" with value "<<debug2[x]<<endl;
                uint32_t check = 0;
                do {
                    //cout<<"skipping x="<<x<<endl;
                    x = ((x >> BlockSizeLog) + 1) << BlockSizeLog;
                    //        cout<<"skipping x to "<<x<<endl;
                    if (x >= otherlarger.Length) {
                        //cout<<"we skipped to the end, aborting"<<endl;
                        goto END_OF_MAIN;
                    }
/*
                    largemainpointer = otherlarger.mainbuffer.data()
                            + otherlarger.highbuffer[x >> BlockSizeLog].second;
                    largemainval = otherlarger.highbuffer[(x >> BlockSizeLog)-1].first;
                    largemainpointer = decode(largemainpointer, largemainval);

                    if (debug2[x] != largemainval) {
                        cout << "I am supposed to be at " << x
                                << " the value there should be " << debug2[x]
                                << endl;
                        cout << "instead I am getting " << largemainval << endl;
                        throw logic_error("no2222");
                        check = x;
                    }
                   cout<<"(in do) I am at "<<x<<" with value "<<debug2[x]<<"==="<<largemainval<<endl;
*/
                } while (otherlarger.highbuffer[x >> BlockSizeLog].first < val);
                largemainpointer = otherlarger.mainbuffer.data()
                        + otherlarger.highbuffer[x >> BlockSizeLog].second;
                largemainval = otherlarger.highbuffer[(x >> BlockSizeLog)-1].first;
                largemainpointer = decode(largemainpointer, largemainval);
                //cout<<"(after do) I am at "<<x<<" with value "<<debug2[x]<<endl;

                if (debug2[x] != largemainval) {
                    cout << "I am supposed to be at " << x
                            << " the value there should be " << debug2[x]
                            << endl;
                    cout << "instead I am getting " << largemainval << endl;
                    cout << "last check at " << check << endl;
                    throw logic_error("no1");
                }
            }
            //cout<<"largemainval="<<largemainval<<endl;
            while (largemainval < val) {
                ++x;
                if (x >= otherlarger.Length) {
//cout<<"We moved to the end"<<endl;
                    goto END_OF_MAIN;
                }
                largemainpointer = decode(largemainpointer, largemainval);
//cout<<"new largemainval="<<largemainval<<endl;
                if(debug2[x]!=largemainval) throw logic_error("no2");
                ///cout<<"(end main) I am at "<<x<<" with value "<<debug2[x]<<endl;
             }
            if (largemainval == val) {
//cout<<"Intersection "<<intersectsize<<" is "<<val<<endl;
                out[intersectsize++] = val;

if(intersectsize>1) if(val == 0) throw runtime_error("bo");
            }
        }
        END_OF_MAIN: return intersectsize;
    }

    uint32_t BlockSizeLog;
    vector<uint8_t> mainbuffer;
    typedef vector<pair<uint32_t, uint32_t>> higharray;
    higharray highbuffer;
    uint32_t Length;
    Skipping(const Skipping & other) : BlockSizeLog(other.BlockSizeLog), mainbuffer(other.mainbuffer),
    highbuffer(other.highbuffer), Length(other.Length) {
if(Length == 0) throw logic_error("fff");
if(mainbuffer.size()==0) throw logic_error("oooooffff");
     }

private:
    // making it private on purpose
    Skipping();
    Skipping & operator=(const Skipping &);

    void load(uint32_t * data, uint32_t length);
    template<uint32_t i>
    uint8_t extract7bits(const uint32_t val) {
        return static_cast<uint8_t> ((val >> (7 * i)) & ((1U << 7) - 1));
    }

    template<uint32_t i>
    uint8_t extract7bitsmaskless(const uint32_t val) {
        return static_cast<uint8_t> ((val >> (7 * i)));
    }
    static inline uint8_t * decode(uint8_t * buffer, uint32_t& prev) {
        //cout<<"decode"<<endl;
        for (uint32_t v = 0, shift = 0;; shift += 7) {
//if(shift/7 >=32) throw logic_error("what?");
            uint8_t c = *buffer++;
            v += ((c & 127) << shift);
            if ((c & 128)) {
                prev += v;
                return buffer;
            }
        }
    }static inline const uint8_t * decode(const uint8_t * buffer, uint32_t& prev) {
        //cout<<"decode"<<endl;
        for (uint32_t v = 0, shift = 0;; shift += 7) {
            uint8_t c = *buffer++;
            v += ((c & 127) << shift);
//cout<<"shift = "<<shift<<" v = "<<v<<endl;
            if ((c & 128)) {
//cout<<"got v = "<<v<<endl;
                prev += v;
                return buffer;
            }
        }
    }
};

void Skipping::load(uint32_t * data, uint32_t len) {
    assert(len < (numeric_limits<size_t>::max() / 5));// check for overflow
    Length = len;
    if(Length == 0) return; // nothing to do
    uint32_t BlockNumber = (Length + (1<<BlockSizeLog) - 1) / (1<<BlockSizeLog);// count full blocks
    assert(BlockNumber << BlockSizeLog >= Length);
    highbuffer.resize(BlockNumber);
    mainbuffer.resize(5 * Length);
    uint8_t * bout = mainbuffer.data();
    uint8_t * const boutinit = bout;
uint32_t sanity = 0;
        uint32_t prev = 0;
    for (uint32_t k = 0; k < BlockNumber; ++k) {
       const uint32_t howmany = (((k + 1)  << BlockSizeLog) > Length) ?
                Length - (k << BlockSizeLog)
                : 1 << BlockSizeLog;
        highbuffer[k] = make_pair(data[(k << BlockSizeLog) + howmany - 1],
                static_cast<uint32_t> (bout - boutinit));
 sanity += howmany;
        for (uint32_t x = 0; x < howmany; ++x) {
            const uint32_t v = data[x + (k << BlockSizeLog)];
//cout<<"v ="<<v<<endl;
const uint32_t val = v - prev;
//cout<<"delta is "<<val<<" prev is "<<prev<<" v="<<v<<endl;
            prev = v;
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
assert(sanity == Length);
    assert(static_cast<uint32_t> (bout - boutinit)>=Length);
    mainbuffer.resize(static_cast<uint32_t> (bout - boutinit));
    mainbuffer.shrink_to_fit();
// check that what we wrote is correct
     bout = mainbuffer.data();

uint32_t val = 0;
for(uint32_t k = 0; k < Length;++k) {
bout = reinterpret_cast<uint8_t * >(decode(bout,val));
//cout<<"got back "<<val<<" expected "<<data[k]<<endl;
if(val != data[k]) throw runtime_error("bug");
}
}

#endif /* SKIPPING_H_ */
