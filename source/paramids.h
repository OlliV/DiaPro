#pragma once

#include <math.h>

#define LOG2DB 8.6858896380650365530225783783321f   // 20 / ln(10)
#define DB2LOG 0.11512925464970228420089957273422f // ln(10) / 20

#define GAIN_MIN               -20.0f
#define GAIN_MAX                20.0f
#define GAIN_DEFAULT_N          0.5f

#define NORM(v, min_v, max_v) \
    ((v) - (min_v)) / ((max_v) - (min_v))

#define PLAIN(v, min_v, max_v) \
    ((v) * ((max_v) - (min_v)) + (min_v))

#define COMP_THRESH_MIN        -60.0f
#define COMP_THRESH_MAX         0.0f
#define COMP_THRESH_DEFAULT_N   0.85f
#define COMP_RATIO_MIN          1.0f
#define COMP_RATIO_MAX          30.0f
#define COMP_RATIO_DEFAULT_N    NORM(3.0f, COMP_RATIO_MIN, COMP_RATIO_MAX)
#define COMP_KNEE_MIN           0.0f
#define COMP_KNEE_MAX           1.0f
#define COMP_KNEE_DEFAULT_N     0.0f
#define COMP_ATTIME_MIN         0.0f
#define COMP_ATTIME_MAX         2000.0f
#define COMP_ATTIME_DEFAULT_N   NORM(10.0f, COMP_ATTIME_MIN, COMP_ATTIME_MAX)
#define COMP_RELTIME_MIN        20.0f
#define COMP_RELTIME_MAX        1000.0f
#define COMP_RELTIME_DEFAULT_N  NORM(50.0f, COMP_RELTIME_MIN, COMP_RELTIME_MAX)
#define COMP_MAKEUP_MIN        -12.0f
#define COMP_MAKEUP_MAX         20.0f
#define COMP_MAKEUP_DEFAULT_N   0.375f
#define COMP_MIX_DEFAULT_N      1.0f
#define DEESSER_THRESH_MIN         -60.0f
#define DEESSER_THRESH_MAX          60.0f
#define DEESSER_THRESH_DEFAULT_N    0.15f
#define DEESSER_FREQ_MIN            1000.0f
#define DEESSER_FREQ_MAX            11100.0f
#define DEESSER_FREQ_DEFAULT_N      0.60f
#define DEESSER_DRIVE_MIN          -20.0f
#define DEESSER_DRIVE_MAX           20.0f
#define DEESSER_DRIVE_DEFAULT_N     0.50f

static inline float db2norm(float db, float min, float max)
{
    return (db - min) / (max - min);
}

static inline float norm2db(float v, float min, float max)
{
    return (v * max) + (min * (1.0f - v));
}

static inline float normdb2factor(float v, float min, float max)
{
    return powf(10.0f, norm2db(v, min, max) / 20.0f);
}

enum {
    /** parameter ID */
    kBypassId = 0,      ///< Bypass value (we will handle the bypass process) (is automatable)
    kVuPPMIn0Id,        ///< for the Vu value return to host (ReadOnly parameter for our UI)
    kVuPPMIn1Id,        ///< for the Vu value return to host (ReadOnly parameter for our UI)
    kVuPPMOut0Id,       ///< for the Vu value return to host (ReadOnly parameter for our UI)
    kVuPPMOut1Id,       ///< for the Vu value return to host (ReadOnly parameter for our UI)
    kGainId,
    kCompThreshId,
    kCompAttimeId,
    kCompReltimeId,
    kCompRatioId,
    kCompKneeId,
    kCompMakeupId,
    kCompMixId,
    kCompStereoLinkId,
    kCompEnabledId,
    kCompGrMeter0Id,
    kCompGrMeter1Id,
    kDeEsserThreshId,
    kDeEsserFreqId,
    kDeEsserDriveId,
    kDeEsserEnabledId,
    kNrParams
};
