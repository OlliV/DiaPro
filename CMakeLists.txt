cmake_minimum_required(VERSION 3.14.0)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "macOS deployment target")
option(SMTG_ADD_VSTGUI "Add VSTGUI Support" ON)

set(vst3sdk_SOURCE_DIR /Users/hbp/repos/vst-dev/VST_SDK/VST3_SDK)
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(DiaPro)
  
set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")
set(SDK_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(DiaPro
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/vst2wrapper/vst2wrapper.sdk.cpp
    source/version.h
    source/cids.h
    source/processor.h
    source/processor.cpp
    source/controller.h
    source/controller.cpp
    source/entry.cpp
)

#- VSTGUI Wanted ----
if(SMTG_ADD_VSTGUI)
	target_sources(DiaPro
		PRIVATE
			resource/editor.uidesc
	)
    target_link_libraries(DiaPro
        PRIVATE
            vstgui_support
    )
    smtg_add_vst3_resource(DiaPro "resource/editor.uidesc")
endif()
# -------------------

smtg_add_vst3_snapshot(DiaPro resource/95C62AD875FF5DFC8D89C9C51E998E0D_snapshot.png)
smtg_add_vst3_snapshot(DiaPro resource/95C62AD875FF5DFC8D89C9C51E998E0D_snapshot_2.0x.png)

if(SMTG_MAC)
    set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE macexport.exp)
endif()

target_link_libraries(DiaPro
    PRIVATE
        sdk
)

if(SMTG_MAC)
    smtg_set_bundle(DiaPro INFOPLIST 
        "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" 
        PREPROCESS
    )
elseif(SMTG_WIN)
    target_sources(DiaPro PRIVATE 
        resource/win32resource.rc
    )
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT DiaPro)
	
	set_target_properties(DiaPro PROPERTIES
		VS_DEBUGGER_COMMAND
			"$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
		VS_DEBUGGER_COMMAND_ARGUMENTS
			" --pluginfolder \"$(OutDir)\" "
	)
endif()
