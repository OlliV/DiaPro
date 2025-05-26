//------------------------------------------------------------------------
// Copyright(c) 2021 Olli Vanhoja.
//------------------------------------------------------------------------

#include "processor.h"
#include "controller.h"
#include "cids.h"
#include "version.h"

#include "public.sdk/source/main/pluginfactory.h"

#define stringPluginName "DiaPro"

/*
 * RFE This is now duplicated somewhere in the SDK?
 */
#if 0
bool InitModule ()
{
	return true;
}

bool DeinitModule ()
{
	return true;
}
#endif

using namespace Steinberg::Vst;
using namespace MyVst;

BEGIN_FACTORY_DEF ("Olli Vanhoja",
			       "https://olli.vanhoja.net",
			       "mailto:olli.vanhoja@gmail.com")

	//---First Plug-in included in this factory-------
	// its kVstAudioEffectClass component
	DEF_CLASS2 (INLINE_UID_FROM_FUID(kDiaProProcessorUID),
				PClassInfo::kManyInstances,	// cardinality
				kVstAudioEffectClass,	// the component category (do not changed this)
				stringPluginName,		// here the Plug-in name
				Vst::kDistributable,	// means that component and controller could be distributed on different computers
				DiaProVST3Category, // Subcategory for this Plug-in
				FULL_VERSION_STR,		// Plug-in version
				kVstVersionString,		// the VST 3 SDK version (do not changed this, use always this define)
				DiaProProcessor::createInstance)	// function pointer called when this component should be instantiated

	// its kVstComponentControllerClass component
	DEF_CLASS2 (INLINE_UID_FROM_FUID (kDiaProControllerUID),
				PClassInfo::kManyInstances, // cardinality
				kVstComponentControllerClass,// the Controller category (do not changed this)
				stringPluginName "Controller",	// controller name (could be the same than component name)
				0,						// not used here
				"",						// not used here
				FULL_VERSION_STR,		// Plug-in version
				kVstVersionString,		// the VST 3 SDK version (do not changed this, use always this define)
				DiaProController::createInstance)// function pointer called when this component should be instantiated

	//----for others Plug-ins contained in this factory, put like for the first Plug-in different DEF_CLASS2---

END_FACTORY
