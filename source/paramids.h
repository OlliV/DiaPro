#pragma once

#include <math.h>

#define LOG2DB 8.6858896380650365530225783783321f   // 20 / ln(10)
#define DB2LOG 0.11512925464970228420089957273422f // ln(10) / 20

#define GAIN_MIN -12.0f
#define GAIN_MAX 12.0f
#define COMP_THRESH_MIN -60.0f

static inline float db2norm(float db, float min, float max)
{
    return (db - min) / (max - min);
}

static inline float norm2db(float v, float min, float max)
{
    return (v * max) + (min * (1 - v));
}

static inline float db2factor(float v, float min, float max)
{
    return powf(10.0f, norm2db(v, min, max) / 20);
}

static inline float norm2factor(float v, float min, float max)
{
    return db2factor(norm2db(v, min, max), min, max);
}

enum {
    /** parameter ID */
    kVuPPMIn0Id = 0,    ///< for the Vu value return to host (ReadOnly parameter for our UI)
    kVuPPMIn1Id,        ///< for the Vu value return to host (ReadOnly parameter for our UI)
    kVuPPMOut0Id,       ///< for the Vu value return to host (ReadOnly parameter for our UI)
    kVuPPMOut1Id,       ///< for the Vu value return to host (ReadOnly parameter for our UI)
    kBypassId,          ///< Bypass value (we will handle the bypass process) (is automatable)
    kGainId,            ///< for the gain value (is automatable)
    kCompThreshId,
    kCompAtttimeId,
    kCompReltimeId,
    kCompRatioId,
    kCompKneeId,
    kCompMixId,
};
