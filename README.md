

- Download and put it somewhere [VST 3 Audio Plug-Ins SDK](https://www.steinberg.net/en/company/developers.html)
- Run `copy_vst2_to_vst3_sdk.sh` in the directory

The latest version of the SDK is missing some files, so download the old version
of the SDK from here [here](https://www.steinberg.net/sdk_downloads/vstsdk366_27_06_2016_build_61.zip)
Copy `plugininterfaces/vst2.x` from the old version to the new version under
`../VST_SDK/VST3_SDK/pluginterfaces`.

Add  `_VSTPluginMain` as the last line of `VST_SDK/VST3_SDK/public.sdk/source/main/macexport.exp`
as the SDK developers forgot to include the main function symbol for VST2.x plug-ins.

Building
--------

Currently the AU version cannot be built and it's disabled.

```
mkdir build
cmake -DSMTG_RUN_VST_VALIDATOR=OFF -DSMTG_ADD_VSTGUI=ON -G"Xcode" -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```
