#pragma once

#include <string.h>

namespace MyVst {
using namespace Steinberg::Vst;

template <typename SampleType, size_t N>
class Delay {
    SampleType ddl[N];
    size_t len;
    size_t ddl_i;
public:
    Delay()
    {
        len = sizeof(ddl) / sizeof(*ddl);
        reset();
    }

    void set_len(size_t nlen)
    {
        size_t olen = len;

        len = std::min(nlen, sizeof(ddl) / sizeof(*ddl));

        // Smoothen out the transition
        if (olen != len) {
            for (size_t i = 0; i < olen; i++) {
                ddl[ddl_i++] = 0.5f * (ddl[ddl_i] * ddl[i]);

                if (ddl_i >= len) {
                    ddl_i = 0;
                }
            }
        }

        if (ddl_i >= len) {
            ddl_i = 0;
        }
    }

    void reset(void)
    {
        ddl_i = 0;
        memset(ddl, 0, sizeof(ddl));
    }

    SampleType process(SampleType xn)
    {
        SampleType yn = ddl[ddl_i];
        ddl[ddl_i++] = xn;

        if (ddl_i >= len) {
            ddl_i = 0;
        }

        return yn;
    }
};
}
