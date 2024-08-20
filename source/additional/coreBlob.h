///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_BLOB_H_
#define _CORE_GUARD_BLOB_H_

#if __has_include(<EGL/egl.h>)

#include <EGL/egl.h>


// ****************************************************************
/* blob-cache definitions */
#if defined(_CORE_WINDOWS_)
    #define CORE_BLOB_LIBRARY_NAME "libEGL.dll"
#elif defined(_CORE_LINUX_)
    #define CORE_BLOB_LIBRARY_NAME "libEGL.so"
#endif

#define CORE_BLOB_CACHE_NAME    (DEFINED(_CORE_DEBUG_) ? "blob_debug.cache" : "blob.cache")   // file name of the blob-cache
#define CORE_BLOB_CACHE_MAGIC   (UINT_LITERAL("CBC0"))                                        // magic number of the blob-cache
#define CORE_BLOB_CACHE_VERSION (0x00000001u)                                                 // current file version of the blob-cache

struct coreCacheEntry final
{
    coreByte*  pData;   // driver-blob data
    coreUint32 iSize;   // size of the data (in bytes)
};

static coreMap<coreUint64, coreCacheEntry> s_aCacheMap   = {};               // driver-blob map (blob-cache)
static coreUint32                          s_iCacheSize  = 0u;               // total size of all data in the map (in bytes)
static coreSpinLock                        s_CacheLock   = coreSpinLock();   // spinlock to prevent concurrent map access
static coreAtomic<coreBool>                s_bCacheState = false;            // dedicated ready state (as changing or removing the callbacks is not possible)

using PFNEGLSETBLOBFUNCANDROIDPROC       = void            (SDLCALL     *) (const void* pKey, khronos_ssize_t iKeySize, const void* pValue, khronos_ssize_t iValueSize);
using PFNEGLGETBLOBFUNCANDROIDPROC       = khronos_ssize_t (SDLCALL     *) (const void* pKey, khronos_ssize_t iKeySize, void*       pValue, khronos_ssize_t iValueSize);
using PFNEGLSETBLOBCACHEFUNCSANDROIDPROC = void            (EGLAPIENTRY *) (EGLDisplay pDisplay, PFNEGLSETBLOBFUNCANDROIDPROC nSetFunc, PFNEGLGETBLOBFUNCANDROIDPROC nGetFunc);


// ****************************************************************
/* calculate blob-cache verification value */
static coreUint32 coreGetBlobCacheCheck()
{
    return coreHashXXH32(PRINT("%s %s %s", glGetString(GL_RENDERER), glGetString(GL_VERSION), CoreApp::Settings::Version));
}


// ****************************************************************
/* set driver-blob */
static void SDLCALL coreSetBlobValue(const void* pKey, khronos_ssize_t iKeySize, const void* pValue, khronos_ssize_t iValueSize)
{
    WARN_IF(!pKey || !iKeySize || !pValue || !iValueSize) return;

    // hash key into simpler form
    const coreUint64 iHash = coreHashXXH64(s_cast<const coreByte*>(pKey), iKeySize);

    const coreSpinLocker oLocker(&s_CacheLock);

    if(s_bCacheState)
    {
        coreCacheEntry& oEntry = s_aCacheMap.bs(iHash);

        // read driver-blob
        if(oEntry.iSize < coreUint32(iValueSize)) DYNAMIC_RESIZE(oEntry.pData, iValueSize)
        std::memcpy(oEntry.pData, pValue, iValueSize);

        // adjust total size
        s_iCacheSize -= oEntry.iSize;
        s_iCacheSize += iValueSize;

        // set properties
        oEntry.iSize = iValueSize;
    }
}


// ****************************************************************
/* get driver-blob */
static khronos_ssize_t SDLCALL coreGetBlobValue(const void* pKey, khronos_ssize_t iKeySize, void* pValue, khronos_ssize_t iValueSize)
{
    WARN_IF(!pKey || !iKeySize || (!pValue && iValueSize)) return 0;

    // hash key into simpler form
    const coreUint64 iHash = coreHashXXH64(s_cast<const coreByte*>(pKey), iKeySize);

    const coreSpinLocker oLocker(&s_CacheLock);

    if(s_bCacheState)
    {
        if(s_aCacheMap.count_bs(iHash))
        {
            const coreCacheEntry& oEntry = s_aCacheMap.at_bs(iHash);

            // write existing driver-blob
            if(oEntry.iSize <= coreUint32(iValueSize)) std::memcpy(pValue, oEntry.pData, oEntry.iSize);

            // always return data size
            return oEntry.iSize;
        }
    }

    return 0;
}


// ****************************************************************
/* load blob-cache from file */
inline coreBool coreLoadBlobCache(const coreChar* pcPath)   // # parameter for internal use
{
    ASSERT(s_aCacheMap.empty() && !s_iCacheSize)

    // load and decompress file
    coreFile oFile(pcPath);
    oFile.Decompress();

    // get file data
    const coreByte* pData   = oFile.GetData();
    const coreByte* pCursor = pData;
    if(!pData)
    {
        Core::Log->Warning("Blob cache could not be loaded");
        return false;
    }

    coreUint32 iMagic;
    coreUint32 iVersion;
    coreUint32 iTotalSize;
    coreUint32 iCheck;
    coreUint32 iNum;

    // read header values
    std::memcpy(&iMagic,     pCursor, sizeof(coreUint32)); pCursor += sizeof(coreUint32);
    std::memcpy(&iVersion,   pCursor, sizeof(coreUint32)); pCursor += sizeof(coreUint32);
    std::memcpy(&iTotalSize, pCursor, sizeof(coreUint32)); pCursor += sizeof(coreUint32);
    std::memcpy(&iCheck,     pCursor, sizeof(coreUint32)); pCursor += sizeof(coreUint32);
    std::memcpy(&iNum,       pCursor, sizeof(coreUint32)); pCursor += sizeof(coreUint32);

    if((iMagic != CORE_BLOB_CACHE_MAGIC) || (iVersion != CORE_BLOB_CACHE_VERSION) || (iTotalSize != oFile.GetSize()) || (iCheck != coreGetBlobCacheCheck()))
    {
        Core::Log->Warning("Blob cache is not valid");
        return false;
    }

    s_CacheLock.Lock();
    {
        for(coreUintW i = 0u, ie = iNum; i < ie; ++i)
        {
            coreUint64     iKey;
            coreCacheEntry oEntry;

            // read entry values
            std::memcpy(&iKey,         pCursor, sizeof(coreUint64)); pCursor += sizeof(coreUint64);
            std::memcpy(&oEntry.iSize, pCursor, sizeof(coreUint32)); pCursor += sizeof(coreUint32);

            // read driver-blob data
            oEntry.pData = NULL; DYNAMIC_RESIZE(oEntry.pData, oEntry.iSize)
            std::memcpy(oEntry.pData, pCursor, oEntry.iSize); pCursor += oEntry.iSize;

            // add entry to map
            s_aCacheMap.emplace_bs(iKey, oEntry);
            s_iCacheSize += oEntry.iSize;
        }
    }
    s_CacheLock.Unlock();

    ASSERT(iTotalSize == coreUint32(pCursor - pData))

    Core::Log->Info("Blob cache loaded (%u entries, %.1f KB)", iNum, I_TO_F(s_iCacheSize) / 1024.0f);
    return true;
}


// ****************************************************************
/* save blob-cache to file */
inline void coreSaveBlobCache()
{
    s_CacheLock.Lock();
    {
        const coreUint32 iTotalSize = 5u * sizeof(coreUint32) + s_aCacheMap.size() * (sizeof(coreUint64) + sizeof(coreUint32)) + s_iCacheSize;

        coreByte* pData   = new coreByte[iTotalSize];
        coreByte* pCursor = pData;

        // prepare header values
        const coreUint32 iMagic   = CORE_BLOB_CACHE_MAGIC;
        const coreUint32 iVersion = CORE_BLOB_CACHE_VERSION;
        const coreUint32 iCheck   = coreGetBlobCacheCheck();
        const coreUint32 iNum     = s_aCacheMap.size();

        // write header values
        std::memcpy(pCursor, &iMagic,     sizeof(coreUint32)); pCursor += sizeof(coreUint32);
        std::memcpy(pCursor, &iVersion,   sizeof(coreUint32)); pCursor += sizeof(coreUint32);
        std::memcpy(pCursor, &iTotalSize, sizeof(coreUint32)); pCursor += sizeof(coreUint32);
        std::memcpy(pCursor, &iCheck,     sizeof(coreUint32)); pCursor += sizeof(coreUint32);
        std::memcpy(pCursor, &iNum,       sizeof(coreUint32)); pCursor += sizeof(coreUint32);

        // loop through all driver-blobs
        FOR_EACH(it, s_aCacheMap)
        {
            const coreUint64 iKey = (*s_aCacheMap.get_key(it));

            // write entry values
            std::memcpy(pCursor, &iKey,      sizeof(coreUint64)); pCursor += sizeof(coreUint64);
            std::memcpy(pCursor, &it->iSize, sizeof(coreUint32)); pCursor += sizeof(coreUint32);
            std::memcpy(pCursor, it->pData,  it->iSize);          pCursor += it->iSize;
        }

        // unlock before writing to disk
        s_CacheLock.Unlock();

        ASSERT(iTotalSize == coreUint32(pCursor - pData))

        // compress and save file
        coreFile oFile(coreData::UserFolderShared(CORE_BLOB_CACHE_NAME), pData, iTotalSize);
        oFile.Compress();
        oFile.Save();

        Core::Log->Info("Blob cache saved (%u entries, %.1f KB)", iNum, I_TO_F(s_iCacheSize) / 1024.0f);
    }
}


// ****************************************************************
/* remove all entries from the blob-cache */
inline void coreClearBlobCache()
{
    const coreSpinLocker oLocker(&s_CacheLock);

    // delete entries
    FOR_EACH(it, s_aCacheMap)
    {
        DYNAMIC_DELETE(it->pData)
    }

    // clear memory
    s_aCacheMap.clear();

    // reset properties
    s_iCacheSize = 0u;
}


// ****************************************************************
/* start up blob-cache */
inline void coreInitBlobCache()
{
    if(s_bCacheState || !Core::Config->GetBool(CORE_CONFIG_GRAPHICS_SHADERCACHE)) return;

    #define __LOAD_FUNCTION(x,y) decltype(x)* __ ## x = r_cast<decltype(x)*>(coreData::GetAddress(y, #x));
    {
        // open EGL library
        void* pLibrary = coreData::OpenLibrary(CORE_BLOB_LIBRARY_NAME);
        if(pLibrary)
        {
            __LOAD_FUNCTION(eglGetCurrentDisplay, pLibrary)
            __LOAD_FUNCTION(eglGetProcAddress,    pLibrary)

            if(__eglGetCurrentDisplay && __eglGetProcAddress)
            {
                // get connection to default display
                EGLDisplay pDisplay = __eglGetCurrentDisplay();
                if(pDisplay)
                {
                    // get function pointer from extension (EGL_ANDROID_blob_cache)
                    const PFNEGLSETBLOBCACHEFUNCSANDROIDPROC eglSetBlobCacheFuncs = r_cast<PFNEGLSETBLOBCACHEFUNCSANDROIDPROC>(__eglGetProcAddress("eglSetBlobCacheFuncsANDROID"));
                    if(eglSetBlobCacheFuncs)
                    {
                        // load blob-cache
                        coreLoadBlobCache(coreData::UserFolderShared(CORE_BLOB_CACHE_NAME));

                        // enable processing
                        s_bCacheState = true;

                        // set callback functions and begin caching
                        eglSetBlobCacheFuncs(pDisplay, coreSetBlobValue, coreGetBlobValue);
                    }
                }
            }

            // close EGL library
            coreData::CloseLibrary(pLibrary);
        }
    }
    #undef __LOAD_FUNCTION
}


// ****************************************************************
/* shut down blob-cache */
inline void coreExitBlobCache()
{
    if(!s_bCacheState || !Core::Config->GetBool(CORE_CONFIG_GRAPHICS_SHADERCACHE)) return;

    // disable processing
    s_bCacheState = false;

    // save and clear blob-cache
    coreSaveBlobCache();
    coreClearBlobCache();
}


#else


// ****************************************************************
/* blob-cache disabled */
inline void coreInitBlobCache() {}
inline void coreExitBlobCache() {}


#endif

#endif /* _CORE_GUARD_BLOB_H_ */