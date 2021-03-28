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

    SampleType act;

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
    } proc[2];
};

template <typename SampleType>
void DeEsser<SampleType>::updateParams(float sampleRate)
{
    cooked.thr = pow(10.0f, 3.0f * thresh - 3.0f);
    cooked.att = 0.010f;
    cooked.rel = 0.992f;
    cooked.fil = 0.050f + 0.94f * freq * freq;
    cooked.gai = 1.0f - normdb2factor(drive, DEESSER_DRIVE_MIN, DEESSER_DRIVE_MAX);
}

template <typename SampleType>
void DeEsser<SampleType>::reset(void)
{
    proc[0].env = proc[0].fbuf1 = proc[0].fbuf2 = 0.0f;
    proc[1].env = proc[1].fbuf1 = proc[1].fbuf2 = 0.0f;
    act = 0.0f;
}

template <typename SampleType>
void DeEsser<SampleType>::process(SampleType **inOut, int nrChannels, int nrSamples)
{
    nrChannels = std::min(nrChannels, 2);
    int act_time = 0;

    for (int ch = 0; ch < nrChannels; ch++) {
        SampleType f1 = proc[ch].fbuf1;
        SampleType f2 = proc[ch].fbuf2;
        SampleType fi = cooked.fil;
        SampleType fo = (1.0f - cooked.fil);
        SampleType at = cooked.att;
        SampleType re = cooked.rel;
        SampleType th = cooked.thr;
        SampleType gg = cooked.gai;
        SampleType en = proc[ch].env;

        SampleType* x = inOut[ch];
        int i = nrSamples;

        while (--i >= 0) {
            SampleType tmp, g;

            tmp = *x;
            f1  = fo * f1 + fi * tmp;
            tmp -= f1;
            f2  = fo * f2 + fi * tmp;
            tmp = gg * (tmp - f2); //extra HF gain

            en = (tmp > en) ? en + at * (tmp - en) : en * re; // envelope
            if (en > th) {
                g = f1 + f2 + tmp * (th / en);

                act_time++;
            } else {
                g = f1 + f2 + tmp; // limit
                //g = 0.5f * (a + b);
            }

            //brackets for full-band!!!
            if (enabled) {
                *x++ = g;
            } else {
                x++;
            }
        }

        if (fabs(f1) < 1.0e-10) {
            proc[ch].fbuf1 = 0.0f;
            proc[ch].fbuf2 = 0.0f;
        } else {
            proc[ch].fbuf1 = f1;
            proc[ch].fbuf2 = f2;
        }
        if (fabs(en) < 1.0e-10) {
            proc[ch].env = 0.0f;
        } else {
            proc[ch].env = en;
        }

        if (enabled && act_time > nrSamples / 2) {
            act = 1.0f;
        } else {
            act *= 0.9995f;
            if (act < 0.1f) {
                act = 0.0f;
            }
        }
    }
}
}
