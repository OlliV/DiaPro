//------------------------------------------------------------------------
// Copyright(c) 2021 Olli Vanhoja.
//------------------------------------------------------------------------

#include "processor.h"
#include "cids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

using namespace Steinberg;

namespace MyVst {
//------------------------------------------------------------------------
// DiaProProcessor
//------------------------------------------------------------------------
DiaProProcessor::DiaProProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kDiaProControllerUID);
}

//------------------------------------------------------------------------
DiaProProcessor::~DiaProProcessor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API DiaProProcessor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instanciated

	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	/* If you don't need an event bus, you can remove the next line */
	//addEventInput (STR16 ("Event In"), 1);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API DiaProProcessor::terminate ()
{
	// Here the Plug-in will be de-instanciated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API DiaProProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API DiaProProcessor::process (Vst::ProcessData& data)
{
	//--- First : Read inputs parameter changes-----------

    /*if (data.inputParameterChanges)
    {
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
        for (int32 index = 0; index < numParamsChanged; index++)
        {
            if (auto* paramQueue = data.inputParameterChanges->getParameterData (index))
            {
                Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount ();
                switch (paramQueue->getParameterId ())
                {
				}
			}
		}
	}*/

	//--- Here you have to implement your processing
    int nrSamples = data.numSamples;
    float *in1 = data.inputs[0].channelBuffers32[0];
    float *in2 = data.inputs[0].channelBuffers32[1];
    float *out1 = data.outputs[0].channelBuffers32[0];
    float *out2 = data.outputs[0].channelBuffers32[1];

    --in1;
    --in2;
    --out1;
    --out2;
    while (nrSamples--) {
        float a = *++in1;
        float b = *++in2;

        float x = 0.5f * (a + b);

        *++out1 = x;
        *++out2 = x;
    }

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API DiaProProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API DiaProProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	/* if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue; */

	return kResultFalse;
}

tresult PLUGIN_API DiaProProcessor::setBusArrangements (Steinberg::Vst::SpeakerArrangement* inputs, int32 numIns,
                                                        Steinberg::Vst::SpeakerArrangement* outputs, int32 numOuts)
{
    if (numIns == 1 && numOuts == 1)
    {
        // the host wants Mono => Mono (or 1 channel -> 1 channel)
        if (Steinberg::Vst::SpeakerArr::getChannelCount (inputs[0]) == 1 &&
            Steinberg::Vst::SpeakerArr::getChannelCount (outputs[0]) == 1)
        {
            auto* bus = FCast<Steinberg::Vst::AudioBus> (audioInputs.at (0));
            if (bus)
            {
                // check if we are Mono => Mono, if not we need to recreate the busses
                if (bus->getArrangement () != inputs[0])
                {
                    getAudioInput (0)->setArrangement (inputs[0]);
                    getAudioInput (0)->setName (STR16 ("Mono In"));
                    getAudioOutput (0)->setArrangement (inputs[0]);
                    getAudioOutput (0)->setName (STR16 ("Mono Out"));
                }
                return kResultOk;
            }
        }
        // the host wants something else than Mono => Mono,
        // in this case we are always Stereo => Stereo
        else
        {
            auto* bus = FCast<Steinberg::Vst::AudioBus> (audioInputs.at (0));
            if (bus)
            {
                tresult result = kResultFalse;

                // the host wants 2->2 (could be LsRs -> LsRs)
                if (Steinberg::Vst::SpeakerArr::getChannelCount (inputs[0]) == 2 &&
                    Steinberg::Vst::SpeakerArr::getChannelCount (outputs[0]) == 2)
                {
                    getAudioInput (0)->setArrangement (inputs[0]);
                    getAudioInput (0)->setName (STR16 ("Stereo In"));
                    getAudioOutput (0)->setArrangement (outputs[0]);
                    getAudioOutput (0)->setName (STR16 ("Stereo Out"));
                    result = kResultTrue;
                }
                // the host want something different than 1->1 or 2->2 : in this case we want stereo
                else if (bus->getArrangement () != Steinberg::Vst::SpeakerArr::kStereo)
                {
                    getAudioInput (0)->setArrangement (Steinberg::Vst::SpeakerArr::kStereo);
                    getAudioInput (0)->setName (STR16 ("Stereo In"));
                    getAudioOutput (0)->setArrangement (Steinberg::Vst::SpeakerArr::kStereo);
                    getAudioOutput (0)->setName (STR16 ("Stereo Out"));
                    result = kResultFalse;
                }

                return result;
            }
        }
    }
    return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API DiaProProcessor::setState (IBStream* state)
{
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer (state, kLittleEndian);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API DiaProProcessor::getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);

	return kResultOk;
}

}
