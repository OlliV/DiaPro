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
             * apply makeup gain
             */
            s *= gain;

            *pSample++ = s;
        }
    }
}

// Copyright 2006, Thomas Scott Stillwell
// All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are permitted
//provided that the following conditions are met:
//
//Redistributions of source code must retain the above copyright notice, this list of conditions
//and the following disclaimer.
//
//Redistributions in binary form must reproduce the above copyright notice, this list of conditions
//and the following disclaimer in the documentation and/or other materials provided with the distribution.
//
//The name of Thomas Scott Stillwell may not be used to endorse or
//promote products derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
//BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
//THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
template <typename SampleType>
void DiaProProcessor::processCompressor (SampleType** inOut, int nrChannels, int nrSamples)
{
    nrChannels = std::min(nrChannels, 2);

    for (int ch = 0; ch < nrChannels; ch++) {
        SampleType *pSample = inOut[ch];
        int i = nrSamples;
        SampleType prevAs = 0;

        while (i--) {
            SampleType s = *pSample;
            SampleType as = fabs(s);
            SampleType amax = fmax(prevAs, as);
            prevAs = as;

            comp.runave[ch] = (amax * amax) + comp.rmscoef * (comp.runave[ch] - amax);
            SampleType det = sqrt(fmax(0, comp.runave[ch]));

            comp.overdb[ch] = 2.08136898f * log(det / comp.cthreshv) * LOG2DB;
            comp.overdb[ch] = fmax(0, comp.overdb[ch]);

            if (comp.overdb[ch] - comp.rundb[ch] > 5) {
                comp.averatio[ch] = 4;
            }

            if (comp.overdb[ch] > comp.rundb[ch]) {
                comp.rundb[ch] = comp.overdb[ch] + comp.atcoef * (comp.rundb[ch] - comp.overdb[ch]);
                comp.runratio[ch] = comp.averatio[ch] + comp.ratatcoef * (comp.runratio[ch] - comp.averatio[ch]);
            } else {
                comp.rundb[ch] = comp.overdb[ch] + comp.relcoef * (comp.rundb[ch] - comp.overdb[ch]);
                comp.runratio[ch] = comp.averatio[ch] + comp.ratrelcoef * (comp.runratio[ch] - comp.averatio[ch]);
            }

            SampleType gr = -comp.overdb[ch] * (comp.cratio - 1) / comp.cratio;
            SampleType grv = exp(gr * DB2LOG);

            comp.runmax[ch] = comp.maxover[ch] + comp.relcoef * (comp.runmax[ch] - comp.maxover[ch]);
            comp.maxover[ch] = comp.runmax[ch];

            if (grv < comp.gr_meter[ch]) {
                comp.gr_meter[ch] = grv;
            } else {
                comp.gr_meter[ch] *= comp.gr_meter_decay;
                if (comp.gr_meter[ch] > 1) {
                    comp.gr_meter[ch] = 1;
                }
            }

            *pSample++ = grv * comp.makeup * comp.mix + s * (1 - comp.mix);
        }
    }
}

template <typename SampleType>
void DiaProProcessor::processVuPPM (SampleType** in, float *vuPPM, int nrChannels, int nrSamples)
{
    nrChannels = std::min(nrChannels, 2);

	for (int ch = 0; ch < nrChannels; ch++) {
		SampleType *ptrIn = (SampleType*)in[ch];
        int i = nrSamples;

		while (i--) {
		    SampleType tmp = (*ptrIn++);

			if (tmp > vuPPM[ch]) {
				vuPPM[ch] = tmp;
			}
		}
	}
}

}
