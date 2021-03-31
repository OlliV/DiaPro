//------------------------------------------------------------------------
// Copyright(c) 2021 Olli Vanhoja.
//------------------------------------------------------------------------

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "base/source/fstreamer.h"
#include "controller.h"
#include "cids.h"
#include "paramids.h"
#include "mdaParameter.h"
#include "vstgui/plugin-bindings/vst3editor.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace MyVst {

//------------------------------------------------------------------------
// GainParameter Declaration
// example of custom parameter (overwriting to and fromString)
//------------------------------------------------------------------------
class GainParameter : public Parameter
{
public:
    GainParameter (const char *name, int32 flags, int32 id, float min, float max);

    void toString (ParamValue normValue, String128 string) const SMTG_OVERRIDE;
    bool fromString (const TChar* string, ParamValue& normValue) const SMTG_OVERRIDE;

private:
    float gain_min;
    float gain_max;
};

GainParameter::GainParameter (const char *name, int32 flags, int32 id, float min_db, float max_db)
{
    Steinberg::UString (info.title, USTRINGSIZE (info.title)).assign (USTRING (name));
    Steinberg::UString (info.units, USTRINGSIZE (info.units)).assign (USTRING ("dB"));

    gain_min = min_db;
    gain_max = max_db;

    info.flags = flags;
    info.id = id;
    info.stepCount = 0;
    info.defaultNormalizedValue = db2norm(0.0f, min_db, max_db);
    info.unitId = kRootUnitId;

    setNormalized(info.defaultNormalizedValue);
}

void GainParameter::toString (ParamValue normValue, String128 string) const
{
    char text[32];
    float db = norm2db(normValue, gain_min, gain_max);

    snprintf(text, sizeof(text), "%.2f", db);
    text[sizeof(text) - 1] = '\0';

    Steinberg::UString (string, 128).fromAscii(text);
}

bool GainParameter::fromString (const TChar* string, ParamValue& normValue) const
{
    String wrapper ((TChar*)string); // don't know buffer size here!
    double tmp = 0.0;

    if (wrapper.scanFloat(tmp)) {
        normValue = db2norm((float)tmp, gain_min, gain_max);

        return true;
    }
    return false;
}

tresult PLUGIN_API DiaProController::initialize (FUnknown* context)
{
	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk) {
		return result;
	}

    //--- Create Units-------------
    UnitInfo unitInfo;
    Unit* unit;
    Parameter *param;

    // create root only if you want to use the programListId
#if 0
    unitInfo.id = kRootUnitId;    // always for Root Unit
    unitInfo.parentUnitId = kNoParentUnitId;    // always for Root Unit
    Steinberg::UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(USTRING("Root"));
    unitInfo.programListId = kNoProgramListId;
    addUnit(new Unit(unitInfo));
#endif

#if 0
    // compressor
    unitInfo.id = 1;
    unitInfo.parentUnitId = kRootUnitId; // attached to the root unit
    Steinberg::UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(USTRING("Compressor"));
    addUnit(new Unit(unitInfo));

    // de-esser
    unitInfo.id = 2;
    unitInfo.parentUnitId = kRootUnitId; // attached to the root unit
    Steinberg::UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(USTRING("De-Esser"));
    addUnit(new Unit(unitInfo));

    // compressor
    unitInfo.id = 3;
    unitInfo.parentUnitId = kRootUnitId; // attached to the root unit
    Steinberg::UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(USTRING("Exciter"));
    addUnit(new Unit(unitInfo));

    // Generic
    unitInfo.id = 4;
    unitInfo.parentUnitId = kRootUnitId; // attached to the root unit
    Steinberg::UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(USTRING("Generic"));
    addUnit(new Unit(unitInfo));
#endif

    // output gain
    unitInfo.id = 2;
    unitInfo.parentUnitId = kRootUnitId; // attached to the root unit
    Steinberg::UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(USTRING("Output"));
    addUnit(new Unit(unitInfo));

    const int32 stepCountToggle = 1;
    ParamValue defaultVal = 0;
    int32 flags = ParameterInfo::kCanAutomate | ParameterInfo::kIsBypass;
    param = parameters.addParameter(STR16("Bypass"), nullptr, stepCountToggle, defaultVal, flags, kBypassId);
    //param->setUnitID(4);

    addVuMeters();
    addGrMeters();

    // Comp thresh
    param = new GainParameter("Compressor Threshold", ParameterInfo::kCanAutomate, kCompThreshId, COMP_THRESH_MIN, COMP_THRESH_MAX);
    param->setNormalized(COMP_THRESH_DEFAULT_N);
    param->setPrecision(2);
    //param->setUnitID(1);
    parameters.addParameter(param);

    // Comp ratio
    param = new Steinberg::Vst::mda::ScaledParameter(USTRING("Compressor Ratio"), USTRING(":1"), 0, COMP_RATIO_DEFAULT_N, ParameterInfo::kCanAutomate, kCompRatioId, COMP_RATIO_MIN, COMP_RATIO_MAX);
    param->setNormalized(COMP_RATIO_DEFAULT_N);
    param->setPrecision(1);
    //param->setUnitID(1);
    parameters.addParameter(param);

    // Comp knee
	param = new Steinberg::Vst::mda::ScaledParameter(USTRING("Compressor Knee"), USTRING(""), 0, COMP_KNEE_DEFAULT_N, ParameterInfo::kCanAutomate, kCompKneeId, COMP_KNEE_MIN, COMP_KNEE_MAX);
    param->setNormalized(COMP_KNEE_DEFAULT_N);
    param->setPrecision(2);
	//param->setUnitID(1);
    parameters.addParameter(param);

    // Comp attack
	param = new Steinberg::Vst::mda::ScaledParameter(USTRING("Compressor Attack"), USTRING("msec"), 0, COMP_ATTIME_DEFAULT_N, ParameterInfo::kCanAutomate, kCompAttimeId, COMP_ATTIME_MIN, COMP_ATTIME_MAX);
    param->setNormalized(COMP_ATTIME_DEFAULT_N);
    param->setPrecision(0);
	//param->setUnitID(1);
    parameters.addParameter(param);

    // Comp release
	param = new Steinberg::Vst::mda::ScaledParameter(USTRING("Compressor Release"), USTRING("msec"), 0, COMP_RELTIME_DEFAULT_N, ParameterInfo::kCanAutomate, kCompReltimeId, COMP_RELTIME_MIN, COMP_RELTIME_MAX);
    param->setNormalized(COMP_RELTIME_DEFAULT_N);
    param->setPrecision(0);
	//param->setUnitID(1);
    parameters.addParameter(param);

    // Comp gain parameter
    param = new GainParameter("Compressor Makeup Gain", ParameterInfo::kCanAutomate, kCompMakeupId, COMP_MAKEUP_MIN, COMP_MAKEUP_MAX);
    param->setPrecision(2);
    //param->setUnitID(1);
    parameters.addParameter(param);

    // Comp mix
	param = new Steinberg::Vst::mda::ScaledParameter(USTRING("Compressor Mix"), USTRING(":1"), 0.0f, 1.0f, ParameterInfo::kCanAutomate, kCompMixId);
    param->setNormalized(COMP_MIX_DEFAULT_N);
    param->setPrecision(2);
	//param->setUnitID(1);
    parameters.addParameter(param);

    // Comp look-ahead
	param = new Steinberg::Vst::mda::ScaledParameter(USTRING("Compressor Look-Ahead"), USTRING("msec"), 0, COMP_LOOKAHEAD_DEFAULT_N, ParameterInfo::kCanAutomate, kCompLookAheadId, COMP_LOOKAHEAD_MIN, COMP_LOOKAHEAD_MAX);
    param->setNormalized(COMP_LOOKAHEAD_DEFAULT_N);
    param->setPrecision(0);
	//param->setUnitID(1);
    parameters.addParameter(param);

    // Comp stereo link
    parameters.addParameter(STR16("Compressor Stereo Link"), // title
                            STR16("On/Off"), // units
                            stepCountToggle,
                            0, // defaultNormalizedValue
                            Vst::ParameterInfo::kCanAutomate, // flags
                            kCompStereoLinkId, // tag
                            0, // unitID TODO 1
                            STR16("CompStereoLink")); // shortTitle

    // Comp enable
    parameters.addParameter(STR16("Enable Compressor"), // title
                            STR16("On/Off"), // units
                            stepCountToggle,
                            1, // defaultNormalizedValue
                            Vst::ParameterInfo::kCanAutomate, // flags
                            kCompEnabledId, // tag
                            0, // unitID TODO 1
                            STR16("EnComp")); // shortTitle


    // De-esser threshold
    param = new GainParameter("De-esser Threshold", ParameterInfo::kCanAutomate, kDeEsserThreshId, DEESSER_THRESH_MIN, DEESSER_THRESH_MAX);
    param->setPrecision(2);
    //param->setUnitID(2);
    parameters.addParameter(param);

    // De-esser freq
	param = new Steinberg::Vst::mda::ScaledParameter(USTRING("De-esser Freq"), USTRING("Hz"), 0, DEESSER_FREQ_DEFAULT_N, ParameterInfo::kCanAutomate, kDeEsserFreqId, DEESSER_FREQ_MIN, DEESSER_FREQ_MAX);
    param->setNormalized(DEESSER_FREQ_DEFAULT_N);
    param->setPrecision(0);
	//param->setUnitID(2);
    parameters.addParameter(param);

    // De-esser drive
    param = new GainParameter("De-esser Drive", ParameterInfo::kCanAutomate, kDeEsserDriveId, DEESSER_DRIVE_MIN, DEESSER_DRIVE_MAX);
    param->setPrecision(2);
    //param->setUnitID(2);
    parameters.addParameter(param);

    // De-esser enable
    parameters.addParameter(STR16("Enable De-Esser"), // title
                            STR16("On/Off"), // units
                            stepCountToggle,
                            1, // defaultNormalizedValue
                            Vst::ParameterInfo::kCanAutomate, // flags
                            kDeEsserEnabledId, // tag
                            0, // unitID TODO 2
                            STR16("EnDeEsser")); // shortTitle

    // De-esser act led
    parameters.addParameter(STR16("DeEsserAct"), nullptr, 0, 0, ParameterInfo::kIsReadOnly, kDeEsserActId);


    // Exciter drive
    param = new GainParameter("Exciter Drive", ParameterInfo::kCanAutomate, kExciterDriveId, EXCITER_DRIVE_MIN, EXCITER_DRIVE_MAX);
    param->setPrecision(2);
    //param->setUnitID(3);
    parameters.addParameter(param);

    // Exciter fc
	param = new Steinberg::Vst::mda::ScaledParameter(USTRING("Exciter fc"), USTRING("Hz"), 0, EXCITER_FC_DEFAULT_N, ParameterInfo::kCanAutomate, kExciterFcId, EXCITER_FC_MIN, EXCITER_FC_MAX);
    param->setNormalized(EXCITER_FC_DEFAULT_N);
    param->setPrecision(0);
	//param->setUnitID(3);
    parameters.addParameter(param);

    // Exciter saturation
    param = new GainParameter("Exciter Saturation", ParameterInfo::kCanAutomate, kExciterSatId, EXCITER_SAT_MIN, EXCITER_SAT_MAX);
    param->setPrecision(2);
    //param->setUnitID(3);
    parameters.addParameter(param);

    // Exciter blend
    param = new GainParameter("Exciter Blend", ParameterInfo::kCanAutomate, kExciterBlendId, EXCITER_BLEND_MIN, EXCITER_BLEND_MAX);
    param->setPrecision(2);
    //param->setUnitID(3);
    parameters.addParameter(param);

    // Exciter enable
    parameters.addParameter(STR16("Enable Exciter"), // title
                            STR16("On/Off"), // units
                            stepCountToggle,
                            1, // defaultNormalizedValue
                            Vst::ParameterInfo::kCanAutomate, // flags
                            kExciterEnabledId, // tag
                            0, // unitID TODO 3
                            STR16("EnExciter")); // shortTitle


    // Output Gain parameter
    param = new GainParameter("Gain", ParameterInfo::kCanAutomate, kGainId, GAIN_MIN, GAIN_MAX);
    param->setPrecision(2);
    //param->setUnitID(4);
    parameters.addParameter(param);

	return result;
}

void DiaProController::addVuMeters(void)
{
    int32 stepCount = 0;
    ParamValue defaultVal = 0;
    int32 flags = ParameterInfo::kIsReadOnly;
    Parameter *param;

    param = parameters.addParameter(STR16("VuPPMIn0"), nullptr, stepCount, defaultVal, flags, kVuPPMIn0Id);
    //param->setUnitID(4);
    param = parameters.addParameter(STR16("VuPPMIn1"), nullptr, stepCount, defaultVal, flags, kVuPPMIn1Id);
    //param->setUnitID(4);
    param = parameters.addParameter(STR16("VuPPMOut0"), nullptr, stepCount, defaultVal, flags, kVuPPMOut0Id);
    //param->setUnitID(4);
    param = parameters.addParameter(STR16("VuPPMOut1"), nullptr, stepCount, defaultVal, flags, kVuPPMOut1Id);
    //param->setUnitID(4);
}

void DiaProController::addGrMeters(void)
{
    int32 stepCount = 0;
    ParamValue defaultVal = 0;
    int32 flags = ParameterInfo::kIsReadOnly;
    Parameter *param;

    param = parameters.addParameter(STR16("CompGrMeter0"), nullptr, stepCount, defaultVal, flags, kCompGrMeter0Id);
    //param->setUnitID(2);
    param = parameters.addParameter(STR16("CompGrMeter1"), nullptr, stepCount, defaultVal, flags, kCompGrMeter1Id);
    //param->setUnitID(2);
}

tresult PLUGIN_API DiaProController::terminate ()
{
	// Here the Plug-in will be de-instanciated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate ();
}

tresult PLUGIN_API DiaProController::setComponentState (IBStream* state)
{
    if (!state)
        return kResultFalse;

    IBStreamer streamer (state, kLittleEndian);
    int32 savedBypass;
    float savedGain;
    float savedCompThresh;
    float savedCompAttime;
    float savedCompReltime;
    float savedCompRatio;
    float savedCompKnee;
    float savedCompMakeup;
    float savedCompMix;
    float savedCompLookAhead;
    int32 savedCompStereoLink;
    int32 savedCompEnabled;
    float savedDeEsserThresh;
    float savedDeEsserFreq;
    float savedDeEsserDrive;
    int32 savedDeEsserEnabled;
    float savedExciterDrive;
    float savedExciterFc;
    float savedExciterSat;
    float savedExciterBlend;
    int32 savedExciterEnabled;

    if (!streamer.readInt32(savedBypass) ||
        !streamer.readFloat(savedGain) ||
        !streamer.readFloat(savedCompThresh) ||
        !streamer.readFloat(savedCompAttime) ||
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

    setParamNormalized(kBypassId, savedBypass ? 1 : 0);
    setParamNormalized(kGainId, savedGain);
    setParamNormalized(kCompThreshId, savedCompThresh);
    setParamNormalized(kCompAttimeId, savedCompAttime);
    setParamNormalized(kCompReltimeId, savedCompReltime);
    setParamNormalized(kCompRatioId, savedCompRatio);
    setParamNormalized(kCompKneeId, savedCompKnee);
    setParamNormalized(kCompMakeupId, savedCompMakeup);
    setParamNormalized(kCompMixId, savedCompMix);
    setParamNormalized(kCompLookAheadId, savedCompLookAhead);
    setParamNormalized(kCompStereoLinkId, savedCompStereoLink ? 1 : 0);
    setParamNormalized(kCompEnabledId, savedCompEnabled ? 1 : 0);
    setParamNormalized(kDeEsserThreshId, savedDeEsserThresh);
    setParamNormalized(kDeEsserFreqId, savedDeEsserFreq);
    setParamNormalized(kDeEsserDriveId, savedDeEsserDrive);
    setParamNormalized(kDeEsserEnabledId, savedDeEsserEnabled ? 1 : 0);
    setParamNormalized(kExciterDriveId, savedExciterDrive);
    setParamNormalized(kExciterFcId, savedExciterFc);
    setParamNormalized(kExciterSatId, savedExciterSat);
    setParamNormalized(kExciterBlendId, savedExciterBlend);
    setParamNormalized(kExciterEnabledId, savedExciterEnabled ? 1 : 0);

	return kResultOk;
}

tresult PLUGIN_API DiaProController::setState (IBStream* state)
{
	// Here you get the state of the controller

	return kResultTrue;
}

tresult PLUGIN_API DiaProController::getState (IBStream* state)
{
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

IPlugView* PLUGIN_API DiaProController::createView (FIDString name)
{
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		// create your editor here and return a IPlugView ptr of it
		auto* view = new VSTGUI::VST3Editor (this, "view", "editor.uidesc");
		return view;
	}
	return nullptr;
}

tresult PLUGIN_API DiaProController::setParamNormalized (Vst::ParamID tag, Vst::ParamValue value)
{
	// called by host to update your parameters
	tresult result = EditControllerEx1::setParamNormalized (tag, value);
	return result;
}

tresult PLUGIN_API DiaProController::getParamStringByValue (Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
	// called by host to get a string for given normalized value of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamStringByValue (tag, valueNormalized, string);
}

tresult PLUGIN_API DiaProController::getParamValueByString (Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
	// called by host to get a normalized value from a string representation of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamValueByString (tag, string, valueNormalized);
}

}

static const Steinberg::Vst::ParamID midi_cc_map[] = {
    [ControllerNumbers::kCtrlVolume] = kGainId,
    [ControllerNumbers::kCtrlGPC2] = kCompThreshId,
    [ControllerNumbers::kCtrlAttackTime] = kCompAttimeId,
    [ControllerNumbers::kCtrlReleaseTime] = kCompReltimeId,
    [ControllerNumbers::kCtrlGPC3] = kCompRatioId,
    [ControllerNumbers::kCtrlGPC4] = kCompKneeId,
    [ControllerNumbers::kCtrlGPC5] = kCompMakeupId,
    [ControllerNumbers::kCtrlPan] = kCompMixId,
    [ControllerNumbers::kCtrlGPC6] = kCompLookAheadId,
#if 0
    [-1] = kCompStereoLinkId,
    [-1] = kCompEnabledId, // Toggles don't really work with the current VST3 SDK version
#endif
    [ControllerNumbers::kCtrlGPC1] = kDeEsserThreshId,
    [ControllerNumbers::kCtrlEffect1] = kDeEsserFreqId,
    [ControllerNumbers::kCtrlEffect2] = kDeEsserDriveId,
#if 0
    [-1] = kDeEsserEnabledId,
#endif
    [ControllerNumbers::kCtrlExpression] = kExciterDriveId,
    [ControllerNumbers::kCtrlFilterCutoff] = kExciterFcId,
    [ControllerNumbers::kCtrlGPC7] = kExciterSatId,
    [ControllerNumbers::kCtrlBalance] = kExciterBlendId,
#if 0
    [-1] = kExciterEnabledId,
#endif
};

tresult PLUGIN_API MyVst::DiaProController::getMidiControllerAssignment (int32 busIndex, int16 midiChannel, Steinberg::Vst::CtrlNumber midiControllerNumber, Steinberg::Vst::ParamID& tag)
{
    Steinberg::Vst::ParamID id;

    if (busIndex != 0 || midiChannel != 1 || midiControllerNumber < 0 || midiControllerNumber > sizeof(midi_cc_map) / sizeof(*midi_cc_map)) {
        return kResultFalse;
    }

    id = midi_cc_map[midiControllerNumber];
    if (id >= 0) {
        tag = id;
        return kResultTrue;
    }

    return kResultFalse;
}
