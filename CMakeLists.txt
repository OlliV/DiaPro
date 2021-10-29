cmake_minimum_required(VERSION 3.14.0)

set(CMAKE_OSX_DEPLOYMENT_TARGET "11.6" CACHE STRING "macOS deployment target")
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "")
option(SMTG_ADD_VSTGUI "Add VSTGUI Support" ON)

set(vst3sdk_SOURCE_DIR ../VST_SDK/VST3_SDK)
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(DiaPro)
set(target DiaPro)
  
set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")
set(SDK_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

add_compile_definitions(HAVE_FFTW)

smtg_add_vst3plugin(${target}
    source/cids.h
    source/controller.cpp
    source/controller.h
    source/entry.cpp
    source/fx/fxobjects.cpp
    source/mdaParameter.cpp
    source/processor.cpp
    source/processor.h
    source/version.h
)

target_sources(${target}
    PRIVATE
    resource/editor.uidesc
    )
target_link_libraries(${target}
    PRIVATE
    vstgui_support
    ${PROJECT_SOURCE_DIR}/fftw/fftw_big_sur/3.3.10/lib/libfftw3.a
    ${PROJECT_SOURCE_DIR}/fftw/fftw_arm64_big_sur/3.3.10/lib/libfftw3.a
    )

smtg_add_vst3_resource(${target} "resource/editor.uidesc")
smtg_add_vst3_resource(${target} "resource/ST_knob_Phase_lll_50x50_128f.png")
smtg_add_vst3_resource(${target} "resource/background.png")
smtg_add_vst3_resource(${target} "resource/switch.png")
smtg_add_vst3_resource(${target} "resource/indicator.png")
smtg_add_vst3_resource(${target} "resource/indicator_rc.png")
smtg_add_vst3_resource(${target} "resource/vu_off.png")
smtg_add_vst3_resource(${target} "resource/vu_on.png")
smtg_add_vst3_resource(${target} "resource/gr_off.png")
smtg_add_vst3_resource(${target} "resource/gr_on.png")

smtg_add_vst3_snapshot(${target} resource/95C62AD875FF5DFC8D89C9C51E998E0D_snapshot.png)
smtg_add_vst3_snapshot(${target} resource/95C62AD875FF5DFC8D89C9C51E998E0D_snapshot_2.0x.png)

if(SMTG_MAC)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE macexport.exp)
    include_directories(AFTER fftw/fftw_big_sur/3.3.10/include)
endif()

target_link_libraries(${target}
    PRIVATE
        sdk
)

if(SMTG_MAC)
    smtg_set_bundle(${target} INFOPLIST 
        "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" 
        PREPROCESS
    )
elseif(SMTG_WIN)
    target_sources(${target} PRIVATE 
        resource/win32resource.rc
    )
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${target})
	
	set_target_properties(${target} PROPERTIES
		VS_DEBUGGER_COMMAND
			"$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
		VS_DEBUGGER_COMMAND_ARGUMENTS
			" --pluginfolder \"$(OutDir)\" "
	)
endif()
