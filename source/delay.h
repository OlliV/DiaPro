#pragma once

#include <string.h>

#define POW2_CEIL(v) (1 + \
(((((((((v) - 1) | (((v) - 1) >> 0x10) | \
      (((v) - 1) | (((v) - 1) >> 0x10) >> 0x08)) | \
     ((((v) - 1) | (((v) - 1) >> 0x10) | \
      (((v) - 1) | (((v) - 1) >> 0x10) >> 0x08)) >> 0x04))) | \
   ((((((v) - 1) | (((v) - 1) >> 0x10) | \
      (((v) - 1) | (((v) - 1) >> 0x10) >> 0x08)) | \
     ((((v) - 1) | (((v) - 1) >> 0x10) | \
      (((v) - 1) | (((v) - 1) >> 0x10) >> 0x08)) >> 0x04))) >> 0x02))) | \
 ((((((((v) - 1) | (((v) - 1) >> 0x10) | \
      (((v) - 1) | (((v) - 1) >> 0x10) >> 0x08)) | \
     ((((v) - 1) | (((v) - 1) >> 0x10) | \
      (((v) - 1) | (((v) - 1) >> 0x10) >> 0x08)) >> 0x04))) | \
   ((((((v) - 1) | (((v) - 1) >> 0x10) | \
      (((v) - 1) | (((v) - 1) >> 0x10) >> 0x08)) | \
     ((((v) - 1) | (((v) - 1) >> 0x10) | \
      (((v) - 1) | (((v) - 1) >> 0x10) >> 0x08)) >> 0x04))) >> 0x02))) >> 0x01))))

namespace MyVst {
using namespace Steinberg::Vst;

template <typename SampleType, size_t N>
class Delay {
    SampleType ddl[POW2_CEIL(N)];
    int len;
    int wr;
    int rd;
public:
    Delay()
    {
        len = sizeof(ddl) / sizeof(*ddl);
        reset();
    }

    void set_len(size_t nlen)
    {
        len = std::min(nlen, sizeof(ddl) / sizeof(*ddl));
    }

    void reset(void)
    {
        wr = 0;
        rd = 0;
        memset(ddl, 0, sizeof(ddl));
    }

    SampleType process(SampleType xn)
    {
        const int mask = sizeof(ddl) / sizeof(*ddl) - 1;

        if (len == 0) {
            return xn;
        }

        wr = (wr + 1) & mask;
        rd = (wr - len) & mask;

        SampleType yn = ddl[rd];
        ddl[wr] = xn;

        return yn;
    }
};
}
