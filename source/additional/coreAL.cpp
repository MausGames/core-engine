///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

coreContextAL g_ContextAL = {};

#define __CORE_ALC_CHECK(x) {g_ContextAL.__ALC_ ## x = alcIsExtensionPresent(pDevice, "ALC_" #x); A = &g_ContextAL.__ALC_ ## x;}
#define __CORE_AL_CHECK(x)  {g_ContextAL.__AL_  ## x = alIsExtensionPresent (         "AL_"  #x); A = &g_ContextAL.__AL_  ## x;}
#define __CORE_ALC_FETCH(f) {if(*A) WARN_IF(!(g_ContextAL.__ ## f = r_cast<decltype(g_ContextAL.__ ## f)>(alcGetProcAddress(pDevice, #f)))) (*A) = false;}
#define __CORE_AL_FETCH(f)  {if(*A) WARN_IF(!(g_ContextAL.__ ## f = r_cast<decltype(g_ContextAL.__ ## f)>(alGetProcAddress (         #f)))) (*A) = false;}


// ****************************************************************
/* init OpenAL */
void coreInitOpenALDevice(ALCdevice* pDevice)
{
    coreBool* A = NULL;

    // reset context structure
    std::memset(&g_ContextAL, 0, sizeof(g_ContextAL));

    // check OpenAL state
    ASSERT(!alcGetCurrentContext())
    WARN_IF(!pDevice) return;

    // implement ALC_SOFT_HRTF
    __CORE_ALC_CHECK(SOFT_HRTF)
    {
        __CORE_ALC_FETCH(alcGetStringiSOFT)
        __CORE_ALC_FETCH(alcResetDeviceSOFT)
    }

    // implement ALC_SOFT_reopen_device
    __CORE_ALC_CHECK(SOFT_reopen_device)
    {
        __CORE_ALC_FETCH(alcReopenDeviceSOFT)
    }

    // implement ALC_SOFT_system_events
    __CORE_ALC_CHECK(SOFT_system_events)
    {
        __CORE_ALC_FETCH(alcEventCallbackSOFT)
        __CORE_ALC_FETCH(alcEventControlSOFT)
    }

    // check remaining ALC extensions
    __CORE_ALC_CHECK(ENUMERATE_ALL_EXT)
    __CORE_ALC_CHECK(EXT_debug)
    __CORE_ALC_CHECK(EXT_disconnect)
    __CORE_ALC_CHECK(SOFT_output_mode)
}

void coreInitOpenALContext()
{
    coreBool* A = NULL;

    // check OpenAL state
    ASSERT(alcGetCurrentContext())

    // implement AL_EXT_debug
    __CORE_AL_CHECK(EXT_debug)
    {
        __CORE_AL_FETCH(alDebugMessageCallbackEXT)
        __CORE_AL_FETCH(alDebugMessageControlEXT)
        __CORE_AL_FETCH(alObjectLabelEXT)
    }

    // implement AL_SOFT_deferred_updates
    __CORE_AL_CHECK(SOFT_deferred_updates)
    {
        __CORE_AL_FETCH(alDeferUpdatesSOFT)
        __CORE_AL_FETCH(alProcessUpdatesSOFT)
    }

    // implement AL_SOFT_source_resampler
    __CORE_AL_CHECK(SOFT_source_resampler)
    {
        __CORE_AL_FETCH(alGetStringiSOFT)
    }

    // check remaining AL extensions
    __CORE_AL_CHECK(EXT_ALAW)
    __CORE_AL_CHECK(EXT_float32)
    __CORE_AL_CHECK(EXT_MULAW)
    __CORE_AL_CHECK(SOFT_buffer_length_query)
    __CORE_AL_CHECK(SOFT_direct_channels)
    __CORE_AL_CHECK(SOFT_direct_channels_remix)
}


// ****************************************************************
/* exit OpenAL */
void coreExitOpenAL()
{
}