#pragma once

#include "fx/fxobjects.h"
#include "paramids.h"

namespace MyVst {
using namespace Steinberg::Vst;

template <typename SampleType>
class Exciter {
public:
    Exciter()
    {
    }

    /*
     * Normalized parameters tuned directly by the user.
     */
    SampleType drive = EXCITER_DRIVE_DEFAULT_N;
    float fc = EXCITER_FC_DEFAULT_N; // fc fo the HPF
    SampleType sat = EXCITER_SAT_DEFAULT_N;
    SampleType blend = EXCITER_BLEND_DEFAULT_N;

    bool enabled = true;

    void updateParams(void);
    void reset(float sampleRate);
    void process(SampleType** inOut, int nrChannels, int nrSamples);
private:
    struct {
        SampleType gdrive;
        SampleType gsat;
    } cooked;

    AudioFilter hpFilter[2]; // Used before the harmonizer
    struct harmonizer {
        Interpolator interpolator;
        Decimator decimator;
        AudioFilter lpFilter; // Used just after the harmonizer
    } harmonizer[2];

    SampleType harmonize(struct harmonizer *h, SampleType xn);
};

template <typename SampleType>
void Exciter<SampleType>::updateParams(void)
{
    // HPF
    for (int i = 0; i < 2; i++) {
        AudioFilterParameters params = hpFilter[i].getParameters();
        params.fc = PLAIN(fc, EXCITER_FC_MIN, EXCITER_FC_MAX);
        hpFilter[i].setParameters(params);
    }

    cooked.gdrive = normdb2factor(drive, EXCITER_DRIVE_MIN, EXCITER_DRIVE_MAX);
    cooked.gsat = normdb2factor(sat, EXCITER_SAT_MIN, EXCITER_SAT_MAX);
}

template <typename SampleType>
void Exciter<SampleType>::reset(float sampleRate)
{
    for (size_t i = 0; i < 2; i++) {
        /*
         * HPF
         */
        AudioFilterParameters hpfParams;
        hpfParams.algorithm = filterAlgorithm::kButterHPF2;
        hpfParams.fc = PLAIN(fc, EXCITER_FC_MIN, EXCITER_FC_MAX);
        hpFilter[i].reset(sampleRate);
        hpFilter[i].setParameters(hpfParams);

        /*
         * Harmonizer
         */
        harmonizer[i].interpolator.initialize(512, rateConversionRatio::k4x, (unsigned int)sampleRate, true);
        harmonizer[i].decimator.initialize(512, rateConversionRatio::k4x, (unsigned int)sampleRate, true);

        AudioFilterParameters lpfParams;
        lpfParams.algorithm = filterAlgorithm::kButterLPF2,
        lpfParams.fc = 19000.0f, // TODO make a define for this
        harmonizer[i].lpFilter.reset(4.0f * sampleRate); // The LPF is used before the decimator
        harmonizer[i].lpFilter.setParameters(lpfParams);
    }
}

template <typename SampleType>
SampleType Exciter<SampleType>::harmonize(struct harmonizer *h, SampleType xn)
{
    InterpolatorOutput interpOut;
    DecimatorInput decimatorIn;

    interpOut = h->interpolator.interpolateAudio(xn);

    for (int i = 0; i < interpOut.count; i++) {
        SampleType s;

        s = interpOut.audioData[i];
        s = softClipWaveShaper(s, cooked.gsat);
        s = h->lpFilter.processAudioSample(s);
        decimatorIn.audioData[i] = s;
    }

    return h->decimator.decimateAudio(decimatorIn);
}

template <typename SampleType>
void Exciter<SampleType>::process(SampleType** inOut, int nrChannels, int nrSamples)
{
    nrChannels = std::min(nrChannels, 2);

    /*
     * This processor is quite CPU intensive, so let's no do anything if
     * it's disabled.
     */
    if (!enabled) {
        return;
    }

    for (int ch = 0; ch < nrChannels; ch++) {
        SampleType *pSample = inOut[ch];
        int i = nrSamples;

        while (i--) {
            SampleType xn, hout;

            xn = *pSample;
            hout = harmonize(&harmonizer[ch], hpFilter[ch].processAudioSample(cooked.gdrive * xn));
            *pSample++ = hout * blend + xn * (1.0f - blend);
        }
    }
}

}
