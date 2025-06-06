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
    //---always initialize the parent-------
    tresult result = AudioEffect::initialize (context);
    if (result != kResultOk) {
        return result;
    }

    //--- create Audio IO ------
    addAudioInput(STR16("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
    addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

    /*
     * We want to grab MIDI CC events from the channel 2 so we need to add
     * two channels and the channel 2 will be in index 2.
     */
    addEventInput(STR16("Event In"), 2);

    return kResultOk;
}

tresult PLUGIN_API DiaProProcessor::terminate ()
{
    //---do not forget to call parent ------
    return AudioEffect::terminate();
}

tresult PLUGIN_API DiaProProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
    if (state) {
        float sampleRate = (float)this->processSetup.sampleRate;

        vuIn.setSampleRate(sampleRate);
        vuOut.setSampleRate(sampleRate);

        dees32.updateParams(sampleRate);
        dees32.reset();
        comp32.updateParams(sampleRate);
        comp32.reset();

        dees64.updateParams(sampleRate);
        dees64.reset();
        comp64.updateParams(sampleRate);
        comp64.reset();

        /*
         * Note that the initialization order is different from the other
         * effect processors.
         */
        exct32.reset(sampleRate);
        exct32.updateParams();

        exct64.reset(sampleRate);
        exct64.updateParams();
    }

    /*
     * Reset VU
     */
    memset(fVuPPMInOld, 0, sizeof(fVuPPMInOld));
    memset(fVuPPMOutOld, 0, sizeof(fVuPPMOutOld));

	return AudioEffect::setActive (state);
}

void DiaProProcessor::handleParamChanges(IParameterChanges* paramChanges)
{
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
	int32 numParamsChanged = paramChanges->getParameterCount ();
    bool compChanged = false;
    bool deesChanged = false;
    bool exctChanged = false;

    // for each parameter which are some changes in this audio block:
    for (int32 i = 0; i < numParamsChanged; i++) {
        IParamValueQueue* paramQueue = paramChanges->getParameterData(i);
		if (paramQueue) {
			ParamValue value;
            int32 sampleOffset;
            int32 numPoints = paramQueue->getPointCount();

            switch (paramQueue->getParameterId ()) {
                case kBypassId:
                    if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) == kResultTrue) {
                        bBypass = value > 0.5f;
                    }
                    break;

                case kGainId:
                    // we use in this example only the last point of the queue.
                    // in some wanted case for specific kind of parameter it makes sense to
                    // retrieve all points and process the whole audio block in small blocks.
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        fGain = value;
                    }
                    break;

                case kCompThreshId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        comp32.thresh = value;
                        comp64.thresh = value;
                        compChanged = true;
                    }
                    break;

                case kCompAttimeId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        comp32.attime = value;
                        comp64.attime = value;
                        compChanged = true;
                    }
                    break;

                case kCompReltimeId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        comp32.reltime = value;
                        comp64.reltime = value;
                        compChanged = true;
                    }
                    break;

                case kCompRatioId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        comp32.ratio = value;
                        comp64.ratio = value;
                        compChanged = true;
                    }
                    break;

                case kCompKneeId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        comp32.knee = value;
                        comp64.knee = value;
                        compChanged = true;
                    }
                    break;

                case kCompMakeupId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        comp32.makeup = value;
                        comp64.makeup = value;
                        compChanged = true;
                    }
                    break;

                case kCompMixId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        comp32.mix = value;
                        comp64.mix = value;
                        compChanged = true;
                    }
                    break;

                case kCompLookAheadId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        comp32.lookahead = value;
                        comp64.lookahead = value;
                        compChanged = true;
                    }
                    break;

                case kCompStereoLinkId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        comp32.stereo_link = value > 0.5f;
                        comp64.stereo_link = value > 0.5f;
                    }
                    break;

                case kCompEnabledId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        comp32.enabled = value > 0.5f;
                        comp64.enabled = value > 0.5f;
                    }
                    break;

                case kDeEsserThreshId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        dees32.thresh = value;
                        dees64.thresh = value;
                        deesChanged = true;
                    }
                    break;

                case kDeEsserFreqId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        dees32.freq = value;
                        dees64.freq = value;
                        deesChanged = true;
                    }
                    break;

                case kDeEsserDriveId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        dees32.drive = value;
                        dees64.drive = value;
                        deesChanged = true;
                    }
                    break;

                case kDeEsserEnabledId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        dees32.enabled = value > 0.5f;
                        dees64.enabled = value > 0.5f;
                    }
                    break;

                case kExciterDriveId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        exct32.drive = value;
                        exct64.drive = value;
                        exctChanged = true;
                    }
                    break;

                case kExciterFcId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        exct32.fc = value;
                        exct64.fc = value;
                        exctChanged = true;
                    }
                    break;

                case kExciterSatId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        exct32.sat = value;
                        exct64.sat = value;
                        exctChanged = true;
                    }
                    break;

                case kExciterBlendId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        exct32.blend = value;
                        exct64.blend = value;
                        exctChanged = true;
                    }
                    break;

                case kExciterEnabledId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
                        exct32.enabled = value > 0.5f;
                        exct64.enabled = value > 0.5f;
                    }
                    break;
            }
        }
    }

    if (compChanged) {
        float sampleRate = (float)this->processSetup.sampleRate;

        comp32.updateParams(sampleRate);
        comp64.updateParams(sampleRate);
    }
    if (deesChanged) {
        float sampleRate = (float)this->processSetup.sampleRate;

        dees32.updateParams(sampleRate);
        dees64.updateParams(sampleRate);
    }
    if (exctChanged) {
        exct32.updateParams();
        exct64.updateParams();
    }
}

tresult PLUGIN_API DiaProProcessor::process (Vst::ProcessData& data)
{
	/*
     * Read inputs parameter changes.
     */
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

    /*
     * We first copy the input bufs to the outputs if necessary.
     */
    size_t sampleFramesSize = getSampleFramesSizeInBytes(processSetup, data.numSamples);
    for (int32 i = 0; i < nrChannels; i++) {
        // in and out might point to the same buffer.
        memmove(out[i], in[i], sampleFramesSize);
    }

    if (data.symbolicSampleSize == kSample32) {
        int nrSamples = data.numSamples;

        vuIn.process<Sample32>((Sample32**)in, nrChannels, nrSamples);

        if (!bBypass) {
            dees32.process((Sample32**)out, nrChannels, nrSamples);
            comp32.process((Sample32**)out, nrChannels, nrSamples);
            exct32.process((Sample32**)out, nrChannels, nrSamples);
            processGain<Sample32>((Sample32**)out, nrChannels, nrSamples, fGain);
        }

        vuOut.process<Sample32>((Sample32**)out, nrChannels, nrSamples);
    } else {
        int nrSamples = data.numSamples;

        vuIn.process<Sample64>((Sample64**)in, nrChannels, nrSamples);

        if (!bBypass) {
            dees64.process((Sample64**)out, nrChannels, nrSamples);
            comp64.process((Sample64**)out, nrChannels, nrSamples);
            exct64.process((Sample64**)out, nrChannels, nrSamples);
            processGain<Sample64>((Sample64**)out, nrChannels, nrSamples, fGain);
        }

        vuOut.process<Sample64>((Sample64**)out, nrChannels, nrSamples);
    }

    /*
     * Write outputs parameter changes
     */
    IParameterChanges* outParamChanges = data.outputParameterChanges;
    /* TODO should we check if VU values changed? */
    if (outParamChanges) {
        int32 index = 0;
        IParamValueQueue* paramQueue;

        paramQueue = outParamChanges->addParameterData(kVuPPMIn0Id, index);
        if (paramQueue) {
            paramQueue->addPoint(0, vuIn.vuPPM[0], index);
        }

        paramQueue = outParamChanges->addParameterData(kVuPPMIn1Id, index);
        if (paramQueue) {
            paramQueue->addPoint(0, vuIn.vuPPM[1], index);
        }

        paramQueue = outParamChanges->addParameterData(kVuPPMOut0Id, index);
        if (paramQueue) {
            paramQueue->addPoint(0, vuOut.vuPPM[0], index);
        }

        paramQueue = outParamChanges->addParameterData(kVuPPMOut1Id, index);
        if (paramQueue) {
            paramQueue->addPoint(0, vuOut.vuPPM[1], index);
        }

        paramQueue = outParamChanges->addParameterData(kCompGrMeter0Id, index);
        if (paramQueue) {
            paramQueue->addPoint(0, (data.symbolicSampleSize == kSample32) ? comp32.gr_meter[0] : comp64.gr_meter[0], index);
        }

        paramQueue = outParamChanges->addParameterData(kCompGrMeter1Id, index);
        if (paramQueue) {
            paramQueue->addPoint(0, (data.symbolicSampleSize == kSample32) ? comp32.gr_meter[1] : comp64.gr_meter[1], index);
        }

        paramQueue = outParamChanges->addParameterData(kDeEsserActId, index);
        if (paramQueue) {
            paramQueue->addPoint(0, (data.symbolicSampleSize == kSample32) ? (Sample32)dees32.act : (Sample64)dees64.act, index);
        }
    }

	return kResultOk;
}

tresult PLUGIN_API DiaProProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing(newSetup);
}

tresult PLUGIN_API DiaProProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	if (symbolicSampleSize == Vst::kSample32 || symbolicSampleSize == Vst::kSample64)
		return kResultTrue;

	return kResultFalse;
}

tresult PLUGIN_API DiaProProcessor::setBusArrangements (Steinberg::Vst::SpeakerArrangement* inputs, int32 numIns,
                                                        Steinberg::Vst::SpeakerArrangement* outputs, int32 numOuts)
{
    if (numIns == 1 && numOuts == 1) {
        // the host wants Mono => Mono (or 1 channel -> 1 channel)
        if (Steinberg::Vst::SpeakerArr::getChannelCount(inputs[0]) == 1 &&
            Steinberg::Vst::SpeakerArr::getChannelCount(outputs[0]) == 1) {
            auto* bus = FCast<Steinberg::Vst::AudioBus>(audioInputs.at (0));
            if (bus) {
                // check if we are Mono => Mono, if not we need to recreate the busses
                if (bus->getArrangement() != Steinberg::Vst::SpeakerArr::kMono) {
                    getAudioInput(0)->setArrangement(inputs[0]);
                    getAudioInput(0)->setName(STR16("Mono In"));
                    // TODO Is this wrong?
                    getAudioOutput(0)->setArrangement(inputs[0]);
                    getAudioOutput(0)->setName (STR16("Mono Out"));
                }
                return kResultOk;
            }
        } else {
            // the host wants something else than Mono => Mono,
            // in this case we are always Stereo => Stereo
            auto* bus = FCast<Steinberg::Vst::AudioBus>(audioInputs.at (0));
            if (bus) {
                tresult result = kResultFalse;

                // the host wants 2->2 (could be LsRs -> LsRs)
                if (Steinberg::Vst::SpeakerArr::getChannelCount(inputs[0]) == 2 &&
                    Steinberg::Vst::SpeakerArr::getChannelCount(outputs[0]) == 2) {
                    getAudioInput (0)->setArrangement(inputs[0]);
                    getAudioInput (0)->setName(STR16("Stereo In"));
                    getAudioOutput (0)->setArrangement(outputs[0]);
                    getAudioOutput (0)->setName(STR16("Stereo Out"));
                    result = kResultTrue;
                } else if (bus->getArrangement() != Steinberg::Vst::SpeakerArr::kStereo) {
                    // the host want something different than 1->1 or 2->2 : in this case we want stereo
                    getAudioInput (0)->setArrangement(Steinberg::Vst::SpeakerArr::kStereo);
                    getAudioInput (0)->setName(STR16("Stereo In"));
                    getAudioOutput (0)->setArrangement(Steinberg::Vst::SpeakerArr::kStereo);
                    getAudioOutput (0)->setName(STR16("Stereo Out"));
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
    float savedComAttime = 0;
    float savedCompReltime = 0;
    float savedCompRatio = 0;
    float savedCompKnee = 0;
    float savedCompMakeup = 0;
    float savedCompMix = 0;
    float savedCompLookAhead = 0;
    int32 savedCompStereoLink = 0;
    int32 savedCompEnabled = 0;
    float savedDeEsserThresh = 0;
    float savedDeEsserFreq = 0;
    float savedDeEsserDrive = 0;
    int32 savedDeEsserEnabled = 0;
    float savedExciterDrive = 0;
    float savedExciterFc = 0;
    float savedExciterSat = 0;
    float savedExciterBlend = 0;
    int32 savedExciterEnabled = 0;

    if (!streamer.readInt32(savedBypass) ||
        !streamer.readFloat(savedGain) ||
        !streamer.readFloat(savedCompThresh) ||
        !streamer.readFloat(savedComAttime) ||
        !streamer.readFloat(savedCompReltime) ||
        !streamer.readFloat(savedCompRatio) ||
        !streamer.readFloat(savedCompKnee) ||
        !streamer.readFloat(savedCompMakeup) ||
        !streamer.readFloat(savedCompMix) ||
        !streamer.readFloat(savedCompLookAhead) ||
        !streamer.readInt32(savedCompStereoLink) ||
        !streamer.readInt32(savedCompEnabled) ||
        !streamer.readFloat(savedDeEsserThresh) ||
        !streamer.readFloat(savedDeEsserFreq) ||
        !streamer.readFloat(savedDeEsserDrive) ||
        !streamer.readInt32(savedDeEsserEnabled) ||
        !streamer.readFloat(savedExciterDrive) ||
        !streamer.readFloat(savedExciterFc) ||
        !streamer.readFloat(savedExciterSat) ||
        !streamer.readFloat(savedExciterBlend) ||
        !streamer.readInt32(savedExciterEnabled)
    ) {
            return kResultFalse;
    }

    bBypass = savedBypass > 0;
    fGain = savedGain;

    float sampleRate = (float)this->processSetup.sampleRate;

    vuIn.setSampleRate(sampleRate);
    vuOut.setSampleRate(sampleRate);

    comp32.thresh = savedCompThresh;
    comp32.attime = savedComAttime;
    comp32.reltime = savedCompReltime;
    comp32.ratio = savedCompRatio;
    comp32.knee = savedCompKnee;
    comp32.makeup = savedCompMakeup;
    comp32.mix = savedCompMix;
    comp32.lookahead = savedCompLookAhead;
    comp32.stereo_link = savedCompStereoLink;
    comp32.enabled = savedCompEnabled;
    comp32.updateParams(sampleRate);

    comp64.thresh = savedCompThresh;
    comp64.attime = savedComAttime;
    comp64.reltime = savedCompReltime;
    comp64.ratio = savedCompRatio;
    comp64.knee = savedCompKnee;
    comp64.makeup = savedCompMakeup;
    comp64.mix = savedCompMix;
    comp64.lookahead = savedCompLookAhead;
    comp64.stereo_link = savedCompStereoLink;
    comp64.enabled = savedCompEnabled;
    comp64.updateParams(sampleRate);

    dees32.thresh = savedDeEsserThresh;
    dees32.freq = savedDeEsserFreq;
    dees32.drive = savedDeEsserDrive;
    dees32.enabled = savedDeEsserEnabled;
    dees32.updateParams(sampleRate);

    dees64.thresh = savedDeEsserThresh;
    dees64.freq = savedDeEsserFreq;
    dees64.drive = savedDeEsserDrive;
    dees64.enabled = savedDeEsserEnabled;
    dees64.updateParams(sampleRate);

    exct32.drive = savedExciterDrive;
    exct32.fc = savedExciterFc;
    exct32.sat = savedExciterSat;
    exct32.blend = savedExciterBlend;
    exct32.enabled = savedExciterEnabled;
    exct32.updateParams();

    exct64.drive = savedExciterDrive;
    exct64.fc = savedExciterFc;
    exct64.sat = savedExciterSat;
    exct64.blend = savedExciterBlend;
    exct64.enabled = savedExciterEnabled;
    exct64.updateParams();

	return kResultOk;
}

tresult PLUGIN_API DiaProProcessor::getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);

    /*
     * YOLO, we just write the sate in the right order here
     * and hope it goes well.
     */
    streamer.writeInt32(bBypass ? 1 : 0);
    streamer.writeFloat(fGain);
    streamer.writeFloat(comp32.thresh);
    streamer.writeFloat(comp32.attime);
    streamer.writeFloat(comp32.reltime);
    streamer.writeFloat(comp32.ratio);
    streamer.writeFloat(comp32.knee);
    streamer.writeFloat(comp32.makeup);
    streamer.writeFloat(comp32.mix);
    streamer.writeFloat(comp32.lookahead);
    streamer.writeInt32(comp32.stereo_link ? 1 : 0);
    streamer.writeInt32(comp32.enabled ? 1 : 0);
    streamer.writeFloat(dees32.thresh);
    streamer.writeFloat(dees32.freq);
    streamer.writeFloat(dees32.drive);
    streamer.writeInt32(dees32.enabled ? 1 : 0);
    streamer.writeFloat(exct32.drive);
    streamer.writeFloat(exct32.fc);
    streamer.writeFloat(exct32.sat);
    streamer.writeFloat(exct32.blend);
    streamer.writeInt32(exct32.enabled ? 1 : 0);

	return kResultOk;
}

}
