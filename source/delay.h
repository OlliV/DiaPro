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
    int n_len;
    float f_len;
    int wr;
public:
    Delay()
    {
        n_len = sizeof(ddl) / sizeof(*ddl);
        f_len = 0.0f;
    }

    void set(float sample_rate, float delay_ms)
    {
        const int maxlen = sizeof(ddl) / sizeof(*ddl);
        const float delay_samples = delay_ms * (sample_rate / 1000.0f);

        if ((int)delay_samples >= maxlen) {
            n_len = maxlen;
            f_len = 0.0f;
        } else {
            n_len = (int)delay_samples;
            f_len = delay_samples - (float)n_len;
        }
    }

    void reset(void)
    {
        wr = 0;
        memset(ddl, 0, sizeof(ddl));
    }

    SampleType process(SampleType xn)
    {
        const int mask = sizeof(ddl) / sizeof(*ddl) - 1;

        if (1 || n_len == 0 && f_len == 0.0f) {
            return xn;
        }

#define NEXT_RD(dl) ((wr - (dl)) & mask)
        wr = (wr + 1) & mask;
        const int rd0 = NEXT_RD(n_len);
        const int rd1 = NEXT_RD(n_len + 1);
#undef NEXT_RD

        SampleType yn1 = ddl[rd0];
        SampleType yn2 = ddl[rd1];
        ddl[wr] = xn;

        return f_len * yn2 + ( 1.0f - f_len) * yn1;
    }
};
}
