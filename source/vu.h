#pragma once

#include <math.h>

namespace MyVst {

class VU {
public:
    float vuPPM[2];

    void setSampleRate(float sampleRate)
    {
        attack = exp(-sampleRate / 0.5e6f);
        release = exp(-sampleRate / 2.5e6f);
    }

    template <typename SampleType>
    void process(SampleType** in, int nrChannels, int nrSamples)
    {
        nrChannels = std::min(nrChannels, 2);

    	for (int ch = 0; ch < nrChannels; ch++) {
            SampleType newPeak = 0;
            SampleType *ptrIn = (SampleType*)in[ch];
            int i = nrSamples;

    		while (i--) {
                SampleType xn = (*ptrIn++);

                if (xn > newPeak) {
                    newPeak = xn;
                }
    		}

            float tmp = vuPPM[ch];
            float decay = newPeak > tmp ? attack : release;
            tmp *= decay;
            tmp += newPeak * (1 - decay);
            vuPPM[ch] = tmp;
    	}
    }

protected:
    float attack;
    float release;
};
}
