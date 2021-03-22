//------------------------------------------------------------------------
// Copyright(c) 2021 Olli Vanhoja.
//------------------------------------------------------------------------

#include "processor.h"
#include "process.h"
#include "cids.h"
#include "paramids.h"

#include "base/source/fstreamer.h"
#include "public.sdk/source/vst/vstaudioprocessoralgo.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

namespace MyVst {
using namespace Steinberg;
using namespace Steinberg::Vst;

DiaProProcessor::DiaProProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kDiaProControllerUID);
}

DiaProProcessor::~DiaProProcessor ()
{}

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

tresult PLUGIN_API DiaProProcessor::terminate ()
{
	// Here the Plug-in will be de-instanciated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return AudioEffect::terminate();
}

void DiaProProcessor::setCompressorParams(float thresh, float attime, float reltime, float ratio, float makeup, float mix)
{
    float sampleRate = (float)this->processSetup.sampleRate;

    comp.thresh = thresh;
    comp.attime = attime;
    comp.reltime = reltime;
    comp.cratio = ratio;
    comp.makeup = makeup;
    comp.mix = 1; // mix original and compressed 0..1
    comp.atcoef = exp(-1 / (comp.attime * sampleRate));
    comp.relcoef = exp(-1 / (comp.reltime * sampleRate));
    float cthresh = (comp.softknee) ? (comp.thresh - 3) : comp.thresh;
    comp.cthreshv = exp(cthresh * DB2LOG);
}

tresult PLUGIN_API DiaProProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
    if (state) {
        float sampleRate = (float)this->processSetup.sampleRate;

        /*
         * Init comp
         */
        comp.softknee = false;
        setCompressorParams(0, 0.010f, 0.100f, 0, 1.0f, 1.0f);
        comp.rmscoef = 0;
        comp.ratatcoef = exp(-1.0f / (0.00001f * sampleRate));
        comp.ratrelcoef = exp(-1.0f / (0.5f * sampleRate));
        comp.gr_meter_decay = exp(1.0f / (1.0f * sampleRate));

        comp.runave[0] = 0;
        comp.runave[1] = 0;
        comp.runmax[0] = 0;
        comp.runmax[1] = 0;
        comp.rundb[0] = 0;
        comp.rundb[1] = 0;
        comp.overdb[0] = 0;
        comp.overdb[1] = 0;
        comp.averatio[0] = 0;
        comp.averatio[1] = 0;
        comp.runratio[0] = 0;
        comp.runratio[1] = 0;
        comp.maxover[0] = 0;
        comp.maxover[1] = 0;
        comp.gr_meter[0] = 1.0f;
        comp.gr_meter[1] = 1.0f;
    }

	return AudioEffect::setActive (state);
}

void DiaProProcessor::handleParamChanges(IParameterChanges* paramChanges)
{
        int32 numParamsChanged = paramChanges->getParameterCount ();
        // for each parameter which are some changes in this audio block:
        for (int32 i = 0; i < numParamsChanged; i++) {
            IParamValueQueue* paramQueue = paramChanges->getParameterData (i);
            if (paramQueue) {
                ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount ();
                switch (paramQueue->getParameterId ()) {
                    case kGainId:
                        // we use in this example only the last point of the queue.
                        // in some wanted case for specific kind of parameter it makes sense to
                        // retrieve all points and process the whole audio block in small blocks.
                        if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                            fGain = norm2factor((float)value, GAIN_MIN, GAIN_MAX);
                        }
                        break;

                    case kCompThreshId:
                        if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                            float thresh = norm2db((float)value, COMP_THRESH_MIN, 0);
                            setCompressorParams(thresh, comp.attime, comp.reltime, comp.cratio, comp.makeup, comp.mix);
                        }
                        break;

                    case kCompAtttimeId:
                        if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                            float attime = 0; // TODO
                            //setCompressorParams(comp.thresh, attime, comp.reltime, comp.cratio, comp.makeup, comp.mix);
                        }
                        break;

                    case kCompReltimeId:
                        break;

                    case kCompRatioId:
                        break;

                    case kCompMixId:
                        break;

                    case kBypassId:
                        if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) == kResultTrue) {
                            bBypass = (value > 0.5f);
                        }
                        break;
                }
            }
        }
}

tresult PLUGIN_API DiaProProcessor::process (Vst::ProcessData& data)
{
	/*
     * Read inputs parameter changes
     */

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
    IParameterChanges* paramChanges = data.inputParameterChanges;
    if (paramChanges) {
        handleParamChanges(paramChanges);
    }

    /*
     * Processing
     */

    if (data.numInputs == 0 || data.numOutputs == 0) {
        return kResultOk;
    }

    int nrChannels = data.inputs[0].numChannels; // We assume that we have the same number of outputs
    void** in = getChannelBuffersPointer(processSetup, data.inputs[0]);
    void** out = getChannelBuffersPointer(processSetup, data.outputs[0]);

    if (data.inputs[0].silenceFlags) {
        size_t sampleFramesSize = getSampleFramesSizeInBytes(processSetup, data.numSamples);

        data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;

        for (int i = 0; i < nrChannels; i++) {
            if (in[i] != out[i]) {
                memset(out[i], 0, sampleFramesSize);
            }
        }

        return kResultOk;
    }

    // Normally the output is not silenced
    data.outputs[0].silenceFlags = 0;

    float fVuPPMIn[2] = { 0 };
    float fVuPPMOut[2] = { 0 };

    /*
    * Process input VU meters
     */
    if (data.symbolicSampleSize == kSample32) {
        processVuPPM<Sample32>((Sample32**)in, fVuPPMIn, nrChannels, data.numSamples);
    } else {
        processVuPPM<Sample64>((Sample64**)in, fVuPPMIn, nrChannels, data.numSamples);
    }

    /*
     * We first copy the input bufs to the outputs if necessary.
     */
    size_t sampleFramesSize = getSampleFramesSizeInBytes(processSetup, data.numSamples);
    for (int32 i = 0; i < nrChannels; i++) {
        // in and out might point to the same buffer.
        memmove(out[i], in[i], sampleFramesSize);
    }

    if (data.symbolicSampleSize == kSample32) {
        if (!bBypass) {
            if (comp.enable) {
                processCompressor<Sample32>((Sample32**)out, nrChannels, data.numSamples);
            }
            processGain<Sample32>((Sample32**)out, nrChannels, data.numSamples, fGain);
        }

        processVuPPM<Sample32>((Sample32**)out, fVuPPMOut, nrChannels, data.numSamples);
    } else {
        if (bBypass) {
            if (comp.enable) {
                processCompressor<Sample64>((Sample64**)out, nrChannels, data.numSamples);
            }
            processGain<Sample64>((Sample64**)out, nrChannels, data.numSamples, fGain);
        }

        processVuPPM<Sample64>((Sample64**)out, fVuPPMOut, nrChannels, data.numSamples);
    }

    /*
     * Write outputs parameter changes
     */
    IParameterChanges* outParamChanges = data.outputParameterChanges;
    /* TODO should we check if VU values changed? */
    if (outParamChanges) {
        int32 index = 0;

        IParamValueQueue* paramQueue0 = outParamChanges->addParameterData(kVuPPMIn0Id, index);
        if (paramQueue0) {
            paramQueue0->addPoint(0, fVuPPMIn[0], index);
        }

        IParamValueQueue* paramQueue1 = outParamChanges->addParameterData(kVuPPMIn1Id, index);
        if (paramQueue1) {
            paramQueue1->addPoint(0, fVuPPMIn[1], index);
        }

        IParamValueQueue* paramQueue2 = outParamChanges->addParameterData(kVuPPMOut0Id, index);
        if (paramQueue2) {
            paramQueue2->addPoint(0, fVuPPMOut[0], index);
        }

        IParamValueQueue* paramQueue3 = outParamChanges->addParameterData(kVuPPMOut1Id, index);
        if (paramQueue3) {
            paramQueue3->addPoint(0, fVuPPMOut[1], index);
        }
    }
    memcpy(fVuPPMOutOld, fVuPPMOut, sizeof(fVuPPMOutOld));

	return kResultOk;
}

tresult PLUGIN_API DiaProProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

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

tresult PLUGIN_API DiaProProcessor::setState (IBStream* state)
{
    // called when we load a preset or project, the model has to be reloaded
    if (!state) {
        kResultFalse;
    }

	IBStreamer streamer(state, kLittleEndian);

    int32 savedBypass = 0;
    float savedGain = 0;
    float savedCompThresh = 0;
    float savedComAtttime;
    float savedCompReltime = 0;
    float savedCompRatio = 0;
    float savedCompKnee = 0;
    float savedCompMakeup = 0;
    float savedCompMix = 0;
    int32 savedCompEnable = 0;

    if (!streamer.readInt32(savedBypass) ||
        !streamer.readFloat(savedGain) ||
        !streamer.readFloat(savedCompThresh) ||
        !streamer.readFloat(savedComAtttime) ||
        !streamer.readFloat(savedCompReltime) ||
        !streamer.readFloat(savedCompRatio) ||
        !streamer.readFloat(savedCompKnee) ||
        !streamer.readFloat(savedCompMakeup) ||
        !streamer.readFloat(savedCompMix) ||
        !streamer.readInt32(savedCompEnable)
        ) {
            return kResultFalse;
    }

    bBypass = savedBypass > 0;
    fGain = savedGain;
    setCompressorParams(savedCompThresh, savedComAtttime, savedCompReltime, savedCompRatio, savedCompMakeup, savedCompMakeup);
    comp.enable = savedCompEnable > 0;

	return kResultOk;
}

tresult PLUGIN_API DiaProProcessor::getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);

	return kResultOk;
}

}
