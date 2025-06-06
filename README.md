DiaPro
======

DiaPro is VST a dialog/vocal processor supporting VST3 API. While the effect is
mostly designed dialog and vocals in mind, there is absolutely no reason why it
couldn't be used for any purpose.

Each unit of the processor is documented separately and at the beginning
of each section there are some presets given as starting point. These
values are determined by trial and error, and by stealing from other
sources such as
[Musician on a mission](https://www.musicianonamission.com/).

![Screenshot showing the plugin](doc/screenshot.jpeg?raw=true "DiaPro")

License
-------

See [LICENSE](LICENSE).


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

Enables the de-esser unit. 

**Threshold [dB]**

Determines the signal levels when the de-esser starts working.

**Frequency [Hz]**

The center frequency of sibilances to be suppressed.

**Drive [dB]**

The gain applied to the sibilances.

### Compressor

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

| Use case    | Ratio     | Knee | Attack [ms] | Release [ms] | Look-Ahead | Makeup | Mix |
|-------------|-----------|------|-------------|--------------|------------|--------|-----|
| Dialog      | 1.5 - 2.0 | 1.00 |      0 - 10 |     80 - 150 |      1 - 5 |    2.0 | 1.0 |
| Vocals      |      1.5  | 0.95 |      2 - 10 |           40 |        >=1 |    2.0 | 1.0 |
| Instruments |           |      |          40 |           60 |          0 |        |     |
| Drums       |           |    0 |             |              |          0 |        | 0.5 |

**Enable**

Enables the compressor unit.

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

### Exciter

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

**Presets for lazy people**

| Use case    | Drive | Freq [Hz] | Saturation |    Mix    |
|-------------|-------|-----------|------------|-----------|
| Dialog      |     6 |      2-5k |       3.20 | 0.2 - 0.5 |

**Enable**

Enables the exciter unit.

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

### Output

**Gain**

Allows adding a final gain to the output.

### MIDI Control

It's possible to control the numeric parameters of this effect with
MIDI Control Commands but it's not possible to toggle the switches
at the moment. All commands must be sent to the MIDI channel 2.

The MIDI control number mapping is as follows:

| Parameters              | MIDI Control Name               | Control no |
|-------------------------|---------------------------------|------------|
| De-esser Threshold      | General Purpose Controller #1   |         16 |
| De-esser Freq           | Effect Control 1                |         12 |
| De-esser Drive          | Effect Control 2                |         13 |
| Compressor Threshold    | General Purpose Controller #2   |         17 |
| Compressor Attack time  | Attack Time                     |         73 |
| Compressor Release time | Release Time                    |         72 |
| Compressor Ratio        | General Purpose Controller #3   |         18 |
| Compressor Knee         | General Purpose Controller #4   |         19 |
| Compressor Makeup       | General Purpose Controller #5   |         80 |
| Compressor Mix          | Pan                             |         10 |
| Compressor Lookahead    | General Purpose Controller #6   |         81 |
| Exciter Drive           | Expression                      |         11 |
| Exciter fc              | Filter Cutoff                   |         71 |
| Exciter Saturation      | General Purpose Controller #7   |         82 |
| Exciter Blend           | Balance                         |          8 |
| Gain                    | Channel Volume                  |          7 |

Building
--------

### Prerequisites

**VST3 SDK**

Download [VST 3 Audio Plug-Ins SDK](https://www.steinberg.net/en/company/developers.html)
and extract it anywhere you wish, but remember that the directory you choose
will be the "installation path" for the SDK.

**fftw**

`fftw` is required by the exciter unit of this plugin effect.

On MacOS you can download `fftw` using the script in the `fftw` directory. This
will download and extract `fftw` for both x86-64 and ARM64 targets.

```
cd fftw
./runme.sh
```

### Running the build

Now you are ready to build the actual plugin.

```
mkdir build
cd build
cmake -DSMTG_RUN_VST_VALIDATOR=OFF -DSMTG_ADD_VSTGUI=ON -GXcode -DCMAKE_BUILD_TYPE=Debug -DCMAKE_OSX_ARCHITECTURES="arm64" -DSMTG_CREATE_PLUGIN_LINK=OFF ..
cmake --build . -t DiaPro
```

or make a release build by running:

```
mkdir build
cd build
cmake -DSMTG_RUN_VST_VALIDATOR=OFF -DSMTG_ADD_VSTGUI=ON -G"Xcode" -DCMAKE_BUILD_TYPE=Release ..
cmake -DSMTG_RUN_VST_VALIDATOR=OFF -DSMTG_ADD_VSTGUI=ON -GXcode -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64" -DSMTG_CREATE_PLUGIN_LINK=OFF ..
cmake --build . --config Release -t DiaPro
```

~~If everything went fine the build script should make a symlink in the system VST
plugin path, pointing to the build result of the build. Meaning that you can now
start your DAW/VST host and it should be able to discover the plugin.~~ 
The symlinking is currently broken and you need to manually copy the VST bundle.

The resulting bundle should work on both x86-64 and ARM64 Macs.

#### Code signing

If you want to sign the binary for distribution the easiest way to sign and
staple the VST3 plugin is shown here.

First you should create an Application Specific Password for the project as
explained [here](https://support.apple.com/en-us/HT204397).
Then run the following command to store the credentials:

```
xcrun notarytool store-credentials --apple-id "youremail" --team-id "team-id"
```

This step needs to be done only once.

Next you need to sign the VST3 plugin:

```
codesign --force -s "Developer ID Application" VST3/Release/DiaPro.vst3
```

Then zip the plugin and run `notarytool` for the zip file:

```
/usr/bin/ditto -c -k --keepParent VST3/Release/DiaPro.vst3 DiaPro.zip
xcrun notarytool submit DiaPro.zip --keychain-profile "DiaPro" --wait
```

If that step passed then you can staple the plugin:

```
 xcrun stapler staple VST3/Release/DiaPro.vst3
```

and remove the intermediate zip:

```
rm DiaPro.zip
```


### Troubleshooting

Some pointers:

**I can't see the plugin in my DAW**

Some DAWs don't look from all possible "legal" VST paths nor it's configurable
You might need to copy the symlink or the bundle/binary to a path that your DAW
supports.

Editing the Interface
---------------------

```
../../VST_SDK/VST3_SDK/build/bin/Debug/editorhost.app/Contents/MacOS/editorhost VST3/Debug/DiaPro.vst3
```

Finally *Save As* on top of `resource/editor.uidesc`. (*Save* should work too,
on the latest SDK version).
