//------------------------------------------------------------------------
// Copyright(c) 2021 Olli Vanhoja.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "paramids.h"
#include "compressor.h"
#include "deesser.h"
#include "exciter.h"
#include "vu.h"

namespace MyVst {

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
    Steinberg::uint32 PLUGIN_API getLatencySamples() override
    {
        /*
         * The compressor is applying a delay even when it's turned off.
         */
        return comp32.get_nlookahead();
    }
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;

	/** For persistence */
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

protected:
    template <typename SampleType>
    void processGain (SampleType** inOut, int nrChannels, int nrSamples, float gain);

    template <typename SampleType>
    void processCompressor (SampleType** inOut, int nrChannels, int nrSamples);

    void handleParamChanges(Steinberg::Vst::IParameterChanges* paramChanges);

    /*
     * Gain
     * Nobody, neither the controller, will pass this if nothing is save, therefore we
     * default it here.
     */
	float fGain = GAIN_DEFAULT_N;

    DeEsser    <Sample32> dees32;
    DeEsser    <Sample64> dees64;
    Compressor <Sample32> comp32;
    Compressor <Sample64> comp64;
    Exciter    <Sample32> exct32;
    Exciter    <Sample64> exct64;

    // VU
    VU vuIn;
    VU vuOut;
	float fVuPPMInOld[2];
	float fVuPPMOutOld[2];

    bool bBypass {false};
};

}
