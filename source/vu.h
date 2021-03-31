#pragma once

#include <math.h>

namespace MyVst {

class VU {
public:
    float vuPPM[2];

    void setSampleRate(float sampleRate)
    {
        decay = exp(-500.0f / sampleRate);
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
    		    SampleType tmp = (*ptrIn++);

    			if (tmp > newPeak) {
    				newPeak = tmp;
                }
    		}
            if (newPeak > vuPPM[ch]) {
                vuPPM[ch] = newPeak;
            } else {
                vuPPM[ch] *= decay;
            }
    	}
    }
protected:
    float decay;
};
}
