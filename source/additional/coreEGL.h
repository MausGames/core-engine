///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_EGL_H_
#define _CORE_GUARD_EGL_H_


// ****************************************************************
/* handle blob-cache */
extern void coreInitBlobCache();
extern void coreExitBlobCache();
extern void coreSaveBlobCache();


// ****************************************************************
/* init EGL */
extern void coreInitEGL();


#endif /* _CORE_GUARD_EGL_H_ */