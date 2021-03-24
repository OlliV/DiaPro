//------------------------------------------------------------------------
// Copyright(c) 2021 Olli Vanhoja.
//------------------------------------------------------------------------

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
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

    // create root only if you want to use the programListId
    //unitInfo.id = kRootUnitId;    // always for Root Unit
    //unitInfo.parentUnitId = kNoParentUnitId;    // always for Root Unit
    //Steinberg::UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(USTRING("Root"));
    //unitInfo.programListId = kNoProgramListId;
    //addUnit(new Unit(unitInfo));

    // compressor
    unitInfo.id = 1;
    unitInfo.parentUnitId = kRootUnitId; // attached to the root unit
    Steinberg::UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(USTRING("Compressor"));
    addUnit(new Unit(unitInfo));

    // output gain
    unitInfo.id = 2;
    unitInfo.parentUnitId = kRootUnitId; // attached to the root unit
    Steinberg::UString(unitInfo.name, USTRINGSIZE(unitInfo.name)).assign(USTRING("Output"));
    addUnit(new Unit(unitInfo));

	// Here you could register some parameters
    addVuMeters();

    int32 stepCount = 1;
    ParamValue defaultVal = 0;
    int32 flags = ParameterInfo::kCanAutomate | ParameterInfo::kIsBypass;
    parameters.addParameter(STR16 ("Bypass"), nullptr, stepCount, defaultVal, flags, kBypassId);

    Parameter *param;

    // Comp thresh
    param = new GainParameter("Threshold", ParameterInfo::kCanAutomate, kCompThreshId, COMP_THRESH_MIN, COMP_THRESH_MAX);
    param->setNormalized(COMP_THRESH_DEFAULT_N);
    parameters.addParameter(param);
    param->setUnitID(1);

    // Comp ratio
    param = new Steinberg::Vst::mda::ScaledParameter(USTRING("Ratio"), USTRING(":1"), 0, COMP_RATIO_DEFAULT_N, ParameterInfo::kCanAutomate, kCompRatioId, COMP_RATIO_MIN, COMP_RATIO_MAX);
    param->setNormalized(COMP_RATIO_DEFAULT_N);
    parameters.addParameter(param);
    param->setUnitID(1);

    // Comp knee
	param = new Steinberg::Vst::mda::ScaledParameter(USTRING("Knee"), USTRING(""), 0, COMP_KNEE_DEFAULT_N, ParameterInfo::kCanAutomate, kCompKneeId, COMP_KNEE_MIN, COMP_KNEE_MAX);
    param->setNormalized(COMP_KNEE_DEFAULT_N);
    parameters.addParameter(param);
	param->setUnitID(1);

    // Comp attack
	param = new Steinberg::Vst::mda::ScaledParameter(USTRING("Attack"), USTRING("usec"), 0, COMP_ATTIME_DEFAULT_N, ParameterInfo::kCanAutomate, kCompAttimeId, COMP_ATTIME_MIN, COMP_ATTIME_MAX);
    param->setNormalized(COMP_ATTIME_DEFAULT_N);
    parameters.addParameter(param);
	param->setUnitID(1);

    // Comp release
	param = new Steinberg::Vst::mda::ScaledParameter(USTRING("Release"), USTRING("msec"), 0, COMP_RELTIME_DEFAULT_N, ParameterInfo::kCanAutomate, kCompReltimeId, COMP_RELTIME_MIN, COMP_RELTIME_MAX);
    param->setNormalized(COMP_RELTIME_DEFAULT_N);
    parameters.addParameter(param);
	param->setUnitID(1);

    // Comp gain parameter
    param = new GainParameter("Makeup Gain", ParameterInfo::kCanAutomate, kCompMakeupId, COMP_MAKEUP_MIN, COMP_MAKEUP_MAX);
    parameters.addParameter(param);
    param->setUnitID(1);

    // Comp mix
	param = new Steinberg::Vst::mda::ScaledParameter(USTRING("Mix"), USTRING(":1"), 0.0f, 1.0f, ParameterInfo::kCanAutomate, kCompMixId);
    param->setNormalized(COMP_MIX_DEFAULT_N);
    parameters.addParameter(param);
	param->setUnitID(1);

    // Comp enable
    parameters.addParameter(STR16("Enable Compressor"), // title
                            STR16("On/Off"), // units
                            1, // stepCount => 1 means toggle
                            1, // defaultNormalizedValue
                            Vst::ParameterInfo::kCanAutomate, // flags
                            kCompEnable, // tag
                            0, // unitID => not using units
                            STR16 ("Enable")); // shortTitle

    // Output Gain parameter
    param = new GainParameter("Gain", ParameterInfo::kCanAutomate, kGainId, GAIN_MIN, GAIN_MAX);
    parameters.addParameter(param);
    param->setUnitID(2);

	return result;
}

void DiaProController::addVuMeters(void)
{
    int32 stepCount = 0;
    ParamValue defaultVal = 0;
    int32 flags = ParameterInfo::kIsReadOnly;

    parameters.addParameter(STR16("VuPPMIn0"), nullptr, stepCount, defaultVal, flags, kVuPPMIn0Id);
    parameters.addParameter(STR16("VuPPMIn1"), nullptr, stepCount, defaultVal, flags, kVuPPMIn1Id);
    parameters.addParameter(STR16("VuPPMOut0"), nullptr, stepCount, defaultVal, flags, kVuPPMOut0Id);
    parameters.addParameter(STR16("VuPPMOut1"), nullptr, stepCount, defaultVal, flags, kVuPPMOut1Id);
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
    int32 savedCompEnable;

    if (!streamer.readInt32(savedBypass) ||
        !streamer.readFloat(savedGain) ||
        !streamer.readFloat(savedCompThresh) ||
        !streamer.readFloat(savedCompAttime) ||
        !streamer.readFloat(savedCompReltime) ||
        !streamer.readFloat(savedCompRatio) ||
        !streamer.readFloat(savedCompKnee) ||
        !streamer.readFloat(savedCompMakeup) ||
        !streamer.readFloat(savedCompMix) ||
        !streamer.readInt32(savedCompEnable)) {
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
    setParamNormalized(kCompEnable, savedCompEnable ? 1 : 0);

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
