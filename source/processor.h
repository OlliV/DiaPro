//------------------------------------------------------------------------
// Copyright(c) 2021 Olli Vanhoja.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"

namespace MyVst {

//------------------------------------------------------------------------
//  DiaProProcessor
//------------------------------------------------------------------------
class DiaProProcessor : public Steinberg::Vst::AudioEffect
{
public:
	DiaProProcessor ();
	~DiaProProcessor () SMTG_OVERRIDE;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/)
	{
		return (Steinberg::Vst::IAudioProcessor*)new DiaProProcessor;
	}

	//--- ---------------------------------------------------------------------
	// AudioEffect overrides:
	//--- ---------------------------------------------------------------------
	/** Called at first after constructor */
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;

	/** Called at the end before destructor */
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;

	/** Switch the Plug-in on/off */
	Steinberg::tresult PLUGIN_API setActive (Steinberg::TBool state) SMTG_OVERRIDE;

	/** Will be called before any process call */
	Steinberg::tresult PLUGIN_API setupProcessing (Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;

	/** Asks if a given sample size is supported see SymbolicSampleSizes. */
	Steinberg::tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

    /** Bus arrangement managing: in this example the 'again' will be mono for mono input/output and
     * stereo for other arrangements. */
    Steinberg::tresult PLUGIN_API setBusArrangements (Steinberg::Vst::SpeakerArrangement* inputs,
                                                      Steinberg::int32 numIns,
                                                      Steinberg::Vst::SpeakerArrangement* outputs,
                                                      Steinberg::int32 numOuts) SMTG_OVERRIDE;

	/** Here we go...the process call */
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;

	/** For persistence */
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

protected:
    template <typename SampleType>
    void processGain (SampleType** inOut, int nrChannels, int nrSamples, float gain);

    template <typename SampleType>
    void processCompressor (SampleType** inOut, int nrChannels, int nrSamples);

    template <typename SampleType>
    void processVuPPM (SampleType** in, float *vuPPM, int nrChannels, int nrSamples);

    void setCompressorParams(float thresh, float attime, float reltime, float ratio, float makeup, float mix);
    void handleParamChanges(Steinberg::Vst::IParameterChanges* paramChanges);

    // Gain
	float fGain;

    // Compressor
    struct {
        bool enable;

        // params
        bool softknee; // TODO Make knee tunable
        float thresh;
        float attime; // attack time
        float reltime; // release time
        float cratio;
        float makeup;
        float mix; // mix original and compressed 0..1
        float rmscoef;
        float atcoef;
        float ratatcoef;
        float relcoef;
        float ratrelcoef;
        float cthreshv;
        float gr_meter_decay;

        // process vars
        float runave[2];
        float runmax[2];
        float rundb[2];
        float overdb[2];
        float averatio[2];
        float runratio[2];
        float maxover[2];
        float gr_meter[2];
    } comp;

    // VU
	float fVuPPMInOld[2];
	float fVuPPMOutOld[2];

    bool bBypass {false};
};

}
