//------------------------------------------------------------------------
// Copyright(c) 2021 Olli Vanhoja.
//------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include "paramids.h"

namespace MyVst {
using namespace Steinberg::Vst;

template <typename SampleType>
class Compressor {
public:
    Compressor()
    {
        thresh = COMP_THRESH_DEFAULT_N;
        attime = COMP_ATTIME_DEFAULT_N;
        reltime = COMP_RELTIME_DEFAULT_N;
        ratio = COMP_RATIO_DEFAULT_N;
        knee = COMP_KNEE_DEFAULT_N;
        makeup = COMP_MAKEUP_DEFAULT_N;
        mix = COMP_MIX_DEFAULT_N;
    };

    SampleType gr_meter[2]; // TODO use for visual

    /*
     * Normalized parameters tuned directly by the user.
     */
    SampleType thresh;
    SampleType attime; // attack time
    SampleType reltime; // release time
    SampleType ratio;
    SampleType knee; // 0.0..1.0
    SampleType makeup;
    SampleType mix; // mix original and compressed 0.0..1.0
    bool enabled = true;

    void updateParams(float sampleRate);
    void reset(void);
    void process(SampleType** inOut, int nrChannels, int nrSamples);
private:

    /**
     * Internal parameters.
     */
    struct {
        SampleType cthresh;
        SampleType rmscoef;
        SampleType atcoef;
        SampleType ratatcoef;
        SampleType relcoef;
        SampleType ratrelcoef;
        SampleType cthreshv;
        SampleType ratio;
        SampleType gr_meter_decay;
        SampleType cmakeup;
    } cooked;

    /**
     * Per channel process variables.
     */
    struct {
        SampleType runave[2];
        SampleType runmax[2];
        SampleType rundb[2];
        SampleType overdb[2];
        SampleType averatio[2];
        SampleType runratio[2];
        SampleType cratio[2];
        SampleType maxover[2];
    } proc;
};

template <typename SampleType>
void Compressor<SampleType>::updateParams(float sampleRate)
{
    cooked.atcoef = exp(-1.0f / (attime * sampleRate));
    cooked.relcoef = exp(-1.0f / (reltime * sampleRate));
    cooked.ratio = PLAIN(ratio, COMP_RATIO_MIN, COMP_RATIO_MAX);
    cooked.cthresh = norm2db(thresh, COMP_THRESH_MIN, COMP_THRESH_MAX) - 3.0f * knee;
    cooked.cthreshv = exp(cooked.cthresh * DB2LOG);
    cooked.cmakeup = norm2db(makeup, COMP_MAKEUP_MIN, COMP_MAKEUP_MAX);

    cooked.rmscoef = 0;
    cooked.ratatcoef = exp(-1.0f / (0.00001f * sampleRate));
    cooked.ratrelcoef = exp(-1.0f / (0.5f * sampleRate));
    cooked.gr_meter_decay = exp(1.0f / (1.0f * sampleRate));
}

template <typename SampleType>
void Compressor<SampleType>::reset(void)
{
    proc.runave[0] = 0;
    proc.runave[1] = 0;
    proc.runmax[0] = 0;
    proc.runmax[1] = 0;
    proc.rundb[0] = 0;
    proc.rundb[1] = 0;
    proc.overdb[0] = 0;
    proc.overdb[1] = 0;
    proc.averatio[0] = 0;
    proc.averatio[1] = 0;
    proc.runratio[0] = 0;
    proc.runratio[1] = 0;
    proc.cratio[0] = 0;
    proc.cratio[1] = 0;
    proc.maxover[0] = 0;
    proc.maxover[1] = 0;
    gr_meter[0] = 1.0f;
    gr_meter[1] = 1.0f;
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
void Compressor<SampleType>::process(SampleType** inOut, int nrChannels, int nrSamples)
{
    nrChannels = std::min(nrChannels, 2);

    for (int ch = 0; ch < nrChannels; ch++) {
        SampleType *pSample = inOut[ch];
        int i = nrSamples;
        SampleType prevAs = 0.0f;

        while (i--) {
            SampleType s = *pSample;
            SampleType as = fabs(s);
            SampleType amax = fmax(prevAs, as);
            prevAs = as;

            proc.runave[ch] = (amax * amax) + cooked.rmscoef * (proc.runave[ch] - amax);
            SampleType det = sqrt(fmax(0.0f, proc.runave[ch]));

            proc.overdb[ch] = 2.08136898f * log(det / cooked.cthreshv) * LOG2DB;
            proc.overdb[ch] = fmax(0.0f, proc.overdb[ch]);

            if (proc.overdb[ch] - proc.rundb[ch] > 5.0f) {
                proc.averatio[ch] = 4.0f;
            }

            if (proc.overdb[ch] > proc.rundb[ch]) {
                proc.rundb[ch] = proc.overdb[ch] + cooked.atcoef * (proc.rundb[ch] - proc.overdb[ch]);
                proc.runratio[ch] = proc.averatio[ch] + cooked.ratatcoef * (proc.runratio[ch] - proc.averatio[ch]);
            } else {
                proc.rundb[ch] = proc.overdb[ch] + cooked.relcoef * (proc.rundb[ch] - proc.overdb[ch]);
                proc.runratio[ch] = proc.averatio[ch] + cooked.ratrelcoef * (proc.runratio[ch] - proc.averatio[ch]);
            }

			proc.overdb[ch] = proc.rundb[ch];
			proc.averatio[ch] = proc.runratio[ch];
			if (ratio == 1.0f) {
				proc.cratio[ch] = 12.0f + proc.averatio[ch];
			} else {
				proc.cratio[ch] = cooked.ratio;
			}

            SampleType gr = -proc.overdb[ch] * (proc.cratio[ch] - 1.0f) / proc.cratio[ch];
            SampleType grv = exp(gr * DB2LOG);

            proc.runmax[ch] = proc.maxover[ch] + cooked.relcoef * (proc.runmax[ch] - proc.maxover[ch]);
            proc.maxover[ch] = proc.runmax[ch];

            if (grv < gr_meter[ch]) {
                gr_meter[ch] = grv;
            } else {
                gr_meter[ch] *= cooked.gr_meter_decay;
                if (gr_meter[ch] > 1.0f) {
                    gr_meter[ch] = 1.0f;
                }
            }

            /*
             * The compressor is always running to make on/off transition smooth
             * and to avoid any sudden glitches when it's enabled.
             */
            if (enabled) {
                *pSample++ = s * grv * cooked.cmakeup * mix + s * (1.0f - mix);
            } else {
                pSample++;
            }
        }
    }
}
}
