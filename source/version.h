//------------------------------------------------------------------------
// Copyright(c) 2021 Olli Vanhoja.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/fplatform.h"

// Here you can define the version of your plug-in: "Major.Sub.Release.Build""
#define MAJOR_VERSION_STR "2"
#define MAJOR_VERSION_INT 2

#define SUB_VERSION_STR "0"
#define SUB_VERSION_INT 0

#define RELEASE_NUMBER_STR "0"
#define RELEASE_NUMBER_INT 0

#define BUILD_NUMBER_STR "2" // Build number to be sure that each result could be identified.
#define BUILD_NUMBER_INT 2

// Version with build number (example "1.0.3.342")
#define FULL_VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR "." BUILD_NUMBER_STR

// Version without build number (example "1.0.3")
#define VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR

#define stringOriginalFilename	"DiaPro.vst3"
#if SMTG_PLATFORM_64
#define stringFileDescription	"DiaPro VST3 (64Bit)"
#else
#define stringFileDescription	"DiaPro VST3"
#endif
#define stringCompanyName		"Olli Vanhoja\0"
#define stringLegalCopyright	"Copyright(c) 2021 Olli Vanhoja."
#define stringLegalTrademarks	"VST is a trademark of Steinberg Media Technologies GmbH"
