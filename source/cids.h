//------------------------------------------------------------------------
// Copyright(c) 2021 Olli Vanhoja.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace MyVst {
static const Steinberg::FUID kDiaProProcessorUID (0x95C62AD8, 0x75FF5DFC, 0x8D89C9C5, 0x1E998E0D);
static const Steinberg::FUID kDiaProControllerUID (0x35E48F4E, 0xB46B5A7B, 0x84800661, 0x9F1499C8);

#define DiaProVST3Category "Fx"
}
