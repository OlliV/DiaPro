/*
*  Copyright 2006, Thomas Scott Stillwell
*  Copyright(c) 2021 Olli Vanhoja.
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted
* provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this list of conditions
* and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions
* and the following disclaimer in the documentation and/or other materials provided with the distribution.
*
* The name of Thomas Scott Stillwell may not be used to endorse or
* promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
* FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <algorithm>
#include "paramids.h"
#include "delay.h"

#define LOOKAHEAD_BUF_SIZE 4096

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
        lookahead = COMP_LOOKAHEAD_DEFAULT_N;
    };

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
    SampleType lookahead; // Used to tune the delay line
    bool stereo_link = false;
    bool enabled = true;

    SampleType gr_meter[2];

    void updateParams(float sampleRate);
    void reset(void);
    unsigned get_nlookahead(void)
    {
        return cooked.nlookahead;
    }
    void process(SampleType** inOut, int nrChannels, int nrSamples);
private:

    /**
     * Internal parameters.
     */
    struct {
        SampleType atcoef;
        SampleType relcoef;
        SampleType cthresh_db;
        SampleType cthreshv;
        SampleType ratio;
        SampleType cmakeup;
        unsigned nlookahead;
        SampleType gr_meter_decay;
    } cooked;

    /**
     * Per channel process variables.
     */
    struct proc {
        Delay <SampleType, LOOKAHEAD_BUF_SIZE> delay;
        SampleType runave;
    } proc[2];

    SampleType detect(struct proc *p, SampleType xn);
    SampleType comp_gr(struct proc *p, SampleType det);
    void update_gr_meter(int ch, SampleType gr);
};

template <typename SampleType>
void Compressor<SampleType>::updateParams(float sampleRate)
{
    cooked.atcoef = exp(-1.0f / ((PLAIN(attime, COMP_ATTIME_MIN, COMP_ATTIME_MAX) / 1000.0f) * sampleRate));
    cooked.relcoef = exp(-1.0f / ((PLAIN(reltime, COMP_RELTIME_MIN, COMP_RELTIME_MAX) / 1000.0f) * sampleRate));
    cooked.ratio = PLAIN(ratio, COMP_RATIO_MIN, COMP_RATIO_MAX);
    cooked.cthresh_db = norm2db(thresh, COMP_THRESH_MIN, COMP_THRESH_MAX);
    cooked.cthreshv = exp(cooked.cthresh_db * DB2LOG);
    cooked.cmakeup = normdb2factor(makeup, COMP_MAKEUP_MIN, COMP_MAKEUP_MAX);

    unsigned nlookahead = (unsigned)(PLAIN(lookahead, COMP_LOOKAHEAD_MIN, COMP_LOOKAHEAD_MAX) * sampleRate);
    proc[0].delay.set_len(nlookahead);
    proc[1].delay.set_len(nlookahead);
    cooked.nlookahead = nlookahead;

    cooked.gr_meter_decay = exp(1.0f / (1.0f * sampleRate));
}

template <typename SampleType>
void Compressor<SampleType>::reset(void)
{
    proc[0].delay.reset();
    proc[1].delay.reset();
    proc[0].runave = 0;
    proc[1].runave = 0;
    gr_meter[0] = 1.0f;
    gr_meter[1] = 1.0f;
}

template <typename SampleType>
SampleType Compressor<SampleType>::detect(struct proc *p, SampleType xn)
{
    const SampleType a = fabs(xn);
    const SampleType det_in = a * a;
    const SampleType rmscoef = det_in > p->runave ? cooked.atcoef : cooked.relcoef;
    p->runave = fmax(0.0f, det_in + rmscoef * (p->runave - det_in));
    const SampleType det = sqrt(p->runave);

    /* det = RMS(in) */
    return det;
}

template <typename SampleType>
SampleType Compressor<SampleType>::comp_gr(struct proc *p, SampleType det)
{
    const SampleType det_db = det <= 0.0f ? -96.0f : log(det) * LOG2DB;
    const SampleType overdb = 2.08136898f * log(det / cooked.cthreshv) * LOG2DB;

    SampleType out_db = 0.0f;
    const SampleType max_knee_width_db = 10.0f;
    const SampleType knee_width_db = norm2db(knee, 0.0f, max_knee_width_db);
    if (overdb < -knee_width_db) {
        /*
         * Left side of the knee, unity gain
         */
        out_db = det_db;
    } else if (fabs(overdb) <= knee_width_db) {
        /*
         * Inside the knee.
         */
        // out_db = det_db + (((1.0f / cooked.ratio) - 1.0) * pow((det_db - cooked.cthresh_db + (knee_width_db / 2.0f)), 2.0f)) / (2.0f * knee_width_db);
        out_db = det_db + (((1.0f / cooked.ratio) - 1.0) * (2.08136898f * (log(det / cooked.cthreshv) + ((knee_width_db / 2) * DB2LOG)) * LOG2DB)) / (2.0f * knee_width_db);
    } else if (overdb > knee_width_db) {
        out_db = cooked.cthresh_db + (det_db - cooked.cthresh_db) / cooked.ratio;
    }

    const SampleType gr_db = out_db - det_db;
    const SampleType gr = exp(gr_db * DB2LOG);

    return gr;
}

template <typename SampleType>
void Compressor<SampleType>::update_gr_meter(int ch, SampleType gr)
{
    if (gr < gr_meter[ch]) {
        gr_meter[ch] = gr;
    } else {
        gr_meter[ch] *= cooked.gr_meter_decay;
        if (gr_meter[ch] > 1.0f) {
            gr_meter[ch] = 1.0f;
        }
    }
}

template <typename SampleType>
void Compressor<SampleType>::process(SampleType** inOut, int nrChannels, int nrSamples)
{
    nrChannels = std::min(nrChannels, 2);

    if (stereo_link && nrChannels == 2) {
        for (int i = 0; i < nrSamples; i++) {
            SampleType xnl = inOut[0][i];
            SampleType xnr = inOut[1][i];
            struct proc *p = &proc[0];

            const SampleType det = detect(p, 0.5 * (xnl + xnr));
            const SampleType gr = comp_gr(p, det);

            /*
             * The compressor is always running to make on/off transition smooth
             * and to avoid any sudden glitches when it's enabled.
             */
            if (enabled) {
                update_gr_meter(0, gr);
                update_gr_meter(1, gr);
                SampleType xnl_dl = proc[0].delay.process(xnl);
                SampleType xnr_dl = proc[1].delay.process(xnr);
                inOut[0][i] = xnl_dl * gr * cooked.cmakeup * mix + xnl_dl * (1.0f - mix);
                inOut[1][i] = xnr_dl * gr * cooked.cmakeup * mix + xnr_dl * (1.0f - mix);
            } else {
                update_gr_meter(0, 1.0f);
                update_gr_meter(1, 1.0f);
            }
        }
    } else {
        for (int ch = 0; ch < nrChannels; ch++) {
            SampleType *pSample = inOut[ch];
            struct proc *p = &proc[ch];
            int i = nrSamples;

            while (i--) {
                SampleType s = *pSample;

                const SampleType det = detect(p, s);
                const SampleType gr = comp_gr(p, det);

                /*
                 * The compressor is always running to make on/off transition smooth
                 * and to avoid any sudden glitches when it's enabled.
                 */
                if (enabled) {
                    update_gr_meter(ch, gr);
                    SampleType xn_dl = p->delay.process(s);
                    *pSample++ = xn_dl * gr * cooked.cmakeup * mix + xn_dl * (1.0f - mix);
                } else {
                    update_gr_meter(ch, 1.0f);

                    /*
                     * We take a sample from the delay line to avoid a
                     * glitch/pop when turning the compressor off and
                     * on again.
                     */
                    *pSample++ = p->delay.process(s);
                }
            }
        }
    }
}
}
