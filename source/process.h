#pragma once

#include <algorithm>
#include "paramids.h"

namespace MyVst {
template <typename SampleType>
void DiaProProcessor::processGain (SampleType** inOut, int nrChannels, int nrSamples, float gain)
{
    for (int ch = 0; ch < nrChannels; ch++) {
        SampleType *pSample = inOut[ch];
        int i = nrSamples;

        while (i--) {
            SampleType s = *pSample;

            /*
             * TODO the gain could be cooked but we need to store the cooked value separately.
             */
            s *= normdb2factor(gain, GAIN_MIN, GAIN_MAX);

            *pSample++ = s;
        }
    }
}
}
