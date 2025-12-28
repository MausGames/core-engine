///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_AL_H_
#define _CORE_GUARD_AL_H_


// ****************************************************************
/* handle all extensions */
#define CORE_ALC_ENUMERATE_ALL_EXT         (__CORE_AL_VAR(ALC_ENUMERATE_ALL_EXT))
#define CORE_ALC_EXT_debug                 (__CORE_AL_VAR(ALC_EXT_debug))
#define CORE_ALC_EXT_disconnect            (__CORE_AL_VAR(ALC_EXT_disconnect))
#define CORE_ALC_SOFT_HRTF                 (__CORE_AL_VAR(ALC_SOFT_HRTF))
#define CORE_ALC_SOFT_output_mode          (__CORE_AL_VAR(ALC_SOFT_output_mode))
#define CORE_ALC_SOFT_reopen_device        (__CORE_AL_VAR(ALC_SOFT_reopen_device))
#define CORE_ALC_SOFT_system_events        (__CORE_AL_VAR(ALC_SOFT_system_events))

#define CORE_AL_EXT_ALAW                   (__CORE_AL_VAR(AL_EXT_ALAW))
#define CORE_AL_EXT_debug                  (__CORE_AL_VAR(AL_EXT_debug))
#define CORE_AL_EXT_float32                (__CORE_AL_VAR(AL_EXT_float32))
#define CORE_AL_EXT_MULAW                  (__CORE_AL_VAR(AL_EXT_MULAW))
#define CORE_AL_SOFT_buffer_length_query   (__CORE_AL_VAR(AL_SOFT_buffer_length_query))
#define CORE_AL_SOFT_deferred_updates      (__CORE_AL_VAR(AL_SOFT_deferred_updates))
#define CORE_AL_SOFT_direct_channels       (__CORE_AL_VAR(AL_SOFT_direct_channels))
#define CORE_AL_SOFT_direct_channels_remix (__CORE_AL_VAR(AL_SOFT_direct_channels_remix))
#define CORE_AL_SOFT_source_resampler      (__CORE_AL_VAR(AL_SOFT_source_resampler))


// ****************************************************************
/* ALC_SOFT_HRTF */
#define alcGetStringiSOFT  __CORE_AL_FUNC(alcGetStringiSOFT)
#define alcResetDeviceSOFT __CORE_AL_FUNC(alcResetDeviceSOFT)


// ****************************************************************
/* ALC_SOFT_reopen_device */
#define alcReopenDeviceSOFT __CORE_AL_FUNC(alcReopenDeviceSOFT)


// ****************************************************************
/* ALC_SOFT_system_events */
#define alcEventCallbackSOFT __CORE_AL_FUNC(alcEventCallbackSOFT)
#define alcEventControlSOFT  __CORE_AL_FUNC(alcEventControlSOFT)


// ****************************************************************
/* AL_EXT_debug */
#define alDebugMessageCallbackEXT __CORE_AL_FUNC(alDebugMessageCallbackEXT)
#define alDebugMessageControlEXT  __CORE_AL_FUNC(alDebugMessageControlEXT)
#define alObjectLabelEXT          __CORE_AL_FUNC(alObjectLabelEXT)


// ****************************************************************
/* AL_SOFT_deferred_updates */
#define alDeferUpdatesSOFT   __CORE_AL_FUNC(alDeferUpdatesSOFT)
#define alProcessUpdatesSOFT __CORE_AL_FUNC(alProcessUpdatesSOFT)


// ****************************************************************
/* AL_SOFT_source_resampler */
#define alGetStringiSOFT __CORE_AL_FUNC(alGetStringiSOFT)


// ****************************************************************
/* context structure */
struct coreContextAL final
{
    coreBool __ALC_ENUMERATE_ALL_EXT;
    coreBool __ALC_EXT_debug;
    coreBool __ALC_EXT_disconnect;
    coreBool __ALC_SOFT_HRTF;
    coreBool __ALC_SOFT_reopen_device;
    coreBool __ALC_SOFT_output_mode;
    coreBool __ALC_SOFT_system_events;

    coreBool __AL_EXT_ALAW;
    coreBool __AL_EXT_debug;
    coreBool __AL_EXT_float32;
    coreBool __AL_EXT_MULAW;
    coreBool __AL_SOFT_buffer_length_query;
    coreBool __AL_SOFT_deferred_updates;
    coreBool __AL_SOFT_direct_channels;
    coreBool __AL_SOFT_direct_channels_remix;
    coreBool __AL_SOFT_source_resampler;

    LPALCGETSTRINGISOFT         __alcGetStringiSOFT;
    LPALCRESETDEVICESOFT        __alcResetDeviceSOFT;
    LPALCREOPENDEVICESOFT       __alcReopenDeviceSOFT;
    LPALCEVENTCALLBACKSOFT      __alcEventCallbackSOFT;
    LPALCEVENTCONTROLSOFT       __alcEventControlSOFT;

    LPALDEBUGMESSAGECALLBACKEXT __alDebugMessageCallbackEXT;
    LPALDEBUGMESSAGECONTROLEXT  __alDebugMessageControlEXT;
    LPALOBJECTLABELEXT          __alObjectLabelEXT;
    LPALDEFERUPDATESSOFT        __alDeferUpdatesSOFT;
    LPALPROCESSUPDATESSOFT      __alProcessUpdatesSOFT;
    LPALGETSTRINGISOFT          __alGetStringiSOFT;
};

extern coreContextAL g_ContextAL;   // context object

#define __CORE_AL_VAR(v)  (g_ContextAL.__ ## v)
#define __CORE_AL_FUNC(f) (g_ContextAL.__ ## f)


// ****************************************************************
/* init and exit OpenAL */
extern void coreInitOpenALDevice(ALCdevice* pDevice);
extern void coreInitOpenALContext();
extern void coreExitOpenAL();


// ****************************************************************
/* check for extensions */
#define CORE_ALC_SUPPORT(e) (CORE_ALC_ ## e)
#define CORE_AL_SUPPORT(e)  (CORE_AL_  ## e)


#endif /* _CORE_GUARD_AL_H_ */