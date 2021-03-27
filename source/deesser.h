/*
 *  Created by Arne Scheffler on 6/14/08.
 *  Copyright (c) 2008 Paul Kellett
 *  Copyright(c) 2021 Olli Vanhoja.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to who  m the Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#pragma once

#include <algorithm>
#include "paramids.h"

namespace MyVst {
using namespace Steinberg::Vst;

template <typename SampleType>
class DeEsser {
public:
    DeEsser()
    {
        thresh = DEESSER_FREQ_DEFAULT_N;
        freq = DEESSER_FREQ_DEFAULT_N;
        drive = DEESSER_DRIVE_DEFAULT_N;
    };

    /*
     * Normalized parameters tuned directly by the user.
     */
    SampleType thresh;
    SampleType freq;
    SampleType drive;
    bool enabled = true;

    void updateParams(float sampleRate);
    void reset(void);
    void process(SampleType** inOut, int nrChannels, int nrSamples);
private:

    /**
     * Internal parameters.
     */
    struct {
        SampleType thr;
        SampleType att;
        SampleType rel;
        SampleType fil;
        SampleType gai;
    } cooked;

    /**
     * Per channel process variables.
     */
    struct proc {
        SampleType env;
        SampleType fbuf1;
        SampleType fbuf2;
    } proc;
};

template <typename SampleType>
void DeEsser<SampleType>::updateParams(float sampleRate)
{
    cooked.thr = pow(10.0f, 3.0f * thresh - 3.0f);
    cooked.att = 0.010f;
    cooked.rel = 0.992f;
    cooked.fil = 0.050f + 0.94f * freq * freq;
    cooked.gai = pow(10.0f, 2.0f * drive - 1.0f);
}

template <typename SampleType>
void DeEsser<SampleType>::reset(void)
{
    proc.env = proc.fbuf1 = proc.fbuf2 = 0.0f;
}

template <typename SampleType>
void DeEsser<SampleType>::process(SampleType **inOut, int nrChannels, int nrSamples)
{
    nrChannels = std::min(nrChannels, 2);

    SampleType* x1 = inOut[0];
    SampleType* x2 = nrChannels == 1 ? inOut[0] : inOut[1];

    SampleType f1 = proc.fbuf1;
    SampleType f2 = proc.fbuf2;
    SampleType fi = cooked.fil;
    SampleType fo = (1.0f - cooked.fil);
    SampleType at = cooked.att;
    SampleType re = cooked.rel;
    SampleType th = cooked.thr;
    SampleType gg = cooked.gai;
    SampleType en = proc.env;

    int i = nrSamples;
    while (--i >= 0) {
        SampleType a, b, tmp, g;

        a = *x1;
        b = *x2;

        tmp = 0.5f * (a + b);
        f1  = fo * f1 + fi * tmp;
        tmp -= f1;
        f2  = fo * f2 + fi * tmp;
        tmp = gg * (tmp - f2); //extra HF gain

        en = (tmp > en) ? en + at * (tmp - en) : en * re; // envelope
        if (en > th) {
            g=f1 + f2 + tmp * (th / en);
        } else {
            g=f1 + f2 + tmp; // limit
        }

        //brackets for full-band!!!
        if (enabled) {
            *x1++ = g;
            *x2++ = g;
        } else {
            x1++;
            x2++;
        }
    }

    if (fabs (f1) < 1.0e-10) {
        proc.fbuf1 = 0.0f;
        proc.fbuf2 = 0.0f;
    } else {
        proc.fbuf1 = f1;
        proc.fbuf2 = f2;
    }
    if (fabs (en) < 1.0e-10) {
        proc.env = 0.0f;
    } else {
        proc.env = en;
    }
}
}
