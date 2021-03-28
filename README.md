DiaPro
======

DiaPro is VST a dialog/vocal processor supporting VST2.x and VST3. While the
effect is mostly designed dialog and vocals in mind, there is absolutely no
reason why it couldn't be used for any purpose.

Each unit of the processor is documented separately and at the beginning
of each section there are some presets given as starting point. These
values are determined by trial and error, and by stealing from other
sources such as
[Musician on a mission](https://www.musicianonamission.com/).

![Screenshot showing the plugin](doc/screenshot.jpeg?raw=true "DiaPro")

Parameters
----------

### De-esser

The de-esser unit is intended to attenuate excess sibilant consonants in a
dialog or vocals.

```
        +-----------+     +------------+        *
        |           |---->| Compressor |---->*     *
x(n) -->| Crossover |     +------------+    *  SUM  *---> y(n)
        |           |----------------------->*     *
        +-----------+                           *
``` 

**Presets for lazy people**

| Use case    | Threshold | Frequency [Hz] | Drive | 
|-------------|-----------|----------------|-------|
| Female      |           |           7-8k |       |
| Male        |           |           5-6k |       |

**Enable**

Enables the de-esser processor. 

**Threshold [dB]**

Determines the signal levels when the de-esser starts working.

**Frequency [Hz]**

The center frequency of sibilances to be suppressed.

**Drive [dB]**

The gain applied to the sibilances.

## Compressor

The compressor unit attenuates loud sounds in the input signal and it's purpose
in this plugin is to improve the clarity, readability and consistence of the
speaker's or singer's voice.

```
                                                               | \
                                    +------------------------->|1-m>----\     *
                                    |                          | /        >*     *
                                    |   | \        | \          |         *  SUM  *---> y(n)
          +----------------------+  |   |    \     |   \       | \        >*     *
x(n) -+-->|         Z^-D         |--+-->| DCA >--->|    >----->| m >----/     *
      |   +----------------------+      |    /     |   /       | /  
      |              |                  | /        | /          |
      |          look-ahead              |          |           |
      |                                  |          |           |
      |   +-------+      +-------+       |          |           |
      |   |  RMS  |      | Gain  |       |          |           |
      +-->|  Det  |----->| Calc  |-------+          |           |
          | =det  |      | =gr   |                  |           |
          +-------+      +-------+                  |           |
            |   |          | | |                    |           |
           att rel         t k n                 makeup        mix
                           h n :                   
                           r e 1
```

**Presets for lazy people**

| Use case    | Ratio    | Knee | Attack [ms] | Release [ms] | Mix  | 
|-------------|----------|------|-------------|--------------|------|
| Vocals      |          |      |      2 - 10 |              |      |
| Instruments |          |      |         40  |      60      |      |
| Drums       |          |    0 |             |              | 0.5  | 

**Enable**

Enables the compressor processor.

**Stereo Link**

Compute the gain reduction from a mix of the left and right channels and then
control the gain of both channels using the same gain reduction value. The
default is to compute the gain reduction separately for each channel.

**Threshold [dB]**

Determines the level that the compressor kicks in. A lower threshold, compresses
the audio more.

**Ratio [n:1]**

The compression ration determines how much the volume is reduced. The ratio of n:1 
means that for every dB that goes above the threshold a 1:n of the dB comes out
from the compressor.

**Knee**

Increasing the knee parameter widens the soft transition range from 1:1 gain to
1:n gain reduction when the input audio is close to the threshold. 0 means there
is no smoothening before the compression kicks in and 1 means that it's set to
the widest setting. Generally making the knee a bit softer makes the compression
sound more subtle.

**Attack time [ms]**

Determines how fast the compressor reacts to a rising signal level.

**Release time [ms]**

Determines how fast the compressor backs off when the signal level goes down.

**Makup gain [ms]**

Increases the output level to compensate the overall signal level reduction.
Typically makeup gain should be adjust the so that the output level is
approximately the same as the input level.

**Look-Ahead [ms]**

The compressor look-ahead delay line length in ms. The delay line allows to
compensate the lag of the detector and thus adjusting the gain reduction of the
output at the right time and not just after the signal already has hit the
threshold.

**Mix [1:m]**

Also known as dry/wet control, allows mixing some of the original audio to the
output.

## Exciter

The exciter unit amplifies the higher frequencies of the incoming audio and
adds harmonics to it by soft-clipping the signal with a waveshaper function.
The soft-clipped signal is finally combined with the original audio.

```
                                                             | \
x(n) -+----------------------------------------------------->|1-n>----\     *
      |                                                      | /        >*     *
      |                                                       |         *  SUM  *---> y(n)
      | | \    +-----+    +-----+  +---+  +-----+  +-----+   | \        >*     *
      +-|   >->| HPF |->->| 4/\ |->| S |->| LPF |->| 4\/ |-->| n >----/     *
        | /    +-----+    +-----+  +---+  +-----+  +-----+   | /
         |        |                  |                        |
         |        |                  |                        |
        drive    fc              saturation                  mix
```

**Drive [dB]**

Gain before the high-pass filter, but it doesn't affect the dry signal used
for the *blend* at the end of the signal chain.

**Freq [Hz]**

High-pass cutoff frequency before the waveshaper.

**Saturation [dB]**

Amplifies the signal after the high pass filter and just before the signal is
feeded into the waveshaper.

**Blend [1:n]**

The function of the blend knob is the same as mix in the compressor. However,
in the case of an exciter it should be kept at minimum, or otherwise only
the frequencies higher than *fc* can be heard in the output.

## Output

**Gain**

Allows adding a final gain to the output.

Building
--------

### Prerequisites

**VST3 SDK**

Download and put it somewhere [VST 3 Audio Plug-Ins SDK](https://www.steinberg.net/en/company/developers.html).
Run `copy_vst2_to_vst3_sdk.sh` in the directory.

The latest version of the SDK is missing some files, so download the old version
of the SDK from here [here](https://www.steinberg.net/sdk_downloads/vstsdk366_27_06_2016_build_61.zip)
Copy `plugininterfaces/vst2.x` from the old version to the new version under
`../VST_SDK/VST3_SDK/pluginterfaces`.

Add  `_VSTPluginMain` as the last line of `VST_SDK/VST3_SDK/public.sdk/source/main/macexport.exp`
as the SDK developers forgot to include the main function symbol for VST2.x plug-ins.

**Install fftw**

`fftw` is used for the exciter unit of this plugin effect. On MacOS it can be
installed using [Brew](https://brew.sh/).

```
brew install fftw
```

### Running the build

```
mkdir build
cmake -DSMTG_RUN_VST_VALIDATOR=OFF -DSMTG_ADD_VSTGUI=ON -G"Xcode" -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

Editing the Interface
---------------------

```
../../VST_SDK/VST3_SDK/build/bin/Debug/editorhost.app/Contents/MacOS/editorhost VST3/Debug/DiaPro.vst3
```

Finally *Save As* on top of `resource/editor.uidesc`.
