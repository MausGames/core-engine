// Modified version for Core Engine
// Please use the original library from the Windows SDK
/******************************************************************
*                                                                 *
*  VersionHelpers.h -- This module defines helper functions to    *
*                      promote version check with proper          *
*                      comparisons.                               *
*                                                                 *
*  Copyright (c) Microsoft Corp.  All rights reserved.            *
*                                                                 *
******************************************************************/
#ifndef _versionhelpers_H_INCLUDED_
#define _versionhelpers_H_INCLUDED_

#ifdef _MSC_VER
    #pragma once
#endif

#ifdef __cplusplus
    #define VERSIONHELPERAPI inline bool
#else
    #define VERSIONHELPERAPI FORCEINLINE BOOL
#endif


VERSIONHELPERAPI IsWindowsVersionOrGreater(DWORD dwMajorVersion, DWORD dwMinorVersion, WORD wServicePackMajor)
{
    OSVERSIONINFOEX oVersionInfo   = {sizeof(oVersionInfo), 0, 0, 0, 0, {0}, 0, 0};
    oVersionInfo.dwMajorVersion    = dwMajorVersion;
    oVersionInfo.dwMinorVersion    = dwMinorVersion;
    oVersionInfo.wServicePackMajor = wServicePackMajor;

    const DWORDLONG iConditionMask = VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0,
                                     VER_MAJORVERSION,     VER_GREATER_EQUAL),
                                     VER_MINORVERSION,     VER_GREATER_EQUAL),
                                     VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

    return (VerifyVersionInfo(&oVersionInfo, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, iConditionMask) != FALSE);
}


#endif // _versionhelpers_H_INCLUDED_
