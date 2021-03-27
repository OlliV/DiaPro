#include "exciter.h"

namespace MyVst {
using namespace Steinberg::Vst;

void Exciter::updateParams(float sampleRate)
{
   harmonizer.interpolator.initialize(512, rateConversionRatio::k4x, (unsigned int)sampleRate, true);
   harmonizer.decimator.initialize(512, rateConversionRatio::k4x, 4 * (unsigned int)sampleRate, true);
}
}
