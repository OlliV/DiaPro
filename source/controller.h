//------------------------------------------------------------------------
// Copyright(c) 2021 Olli Vanhoja.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

namespace MyVst {
using namespace Steinberg;
using namespace Steinberg::Vst;

enum DiaProUnitId {
    DiaProRootUnitId = 0,
    DiaProCompressorUnitId,
    DiaProDeEsserUnitId,
    DiaProExciterUnitId,
    DiaProOutputUnitId,
    DiaProNrUnits
};

//------------------------------------------------------------------------
//  DiaProController
//------------------------------------------------------------------------
class DiaProController : public EditControllerEx1, public IMidiMapping
{
public:
	DiaProController () = default;
	~DiaProController () SMTG_OVERRIDE = default;

    // Create function
	static FUnknown* createInstance (void* /*context*/)
	{
		return (IEditController*)new DiaProController;
	}

	// IPluginBase
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;

    //---IMidiMapping---------------------------
    tresult PLUGIN_API getMidiControllerAssignment (int32 busIndex, int16 channel, CtrlNumber midiControllerNumber, ParamID& id) SMTG_OVERRIDE;

	// EditController
	Steinberg::tresult PLUGIN_API setComponentState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setParamNormalized (ParamID tag,
                                                      ParamValue value) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getParamStringByValue (ParamID tag,
                                                         ParamValue valueNormalized,
                                                         String128 string) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getParamValueByString (ParamID tag,
                                                         TChar* string,
                                                         ParamValue& valueNormalized) SMTG_OVERRIDE;

 	//---Interface---------
	DEFINE_INTERFACES
        DEF_INTERFACE (IMidiMapping)
        DEF_INTERFACE (IUnitInfo)
	END_DEFINE_INTERFACES (EditController)
    DELEGATE_REFCOUNT (EditController)

protected:
private:
    void addVuMeters(void);
    void addGrMeters(void);
};

}
