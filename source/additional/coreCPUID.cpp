///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

alignas(ALIGNMENT_CACHE) const coreCPUID::__coreCPUID coreCPUID::s_CPUID = {};


// ****************************************************************
/* constructor */
coreCPUID::__coreCPUID::__coreCPUID()noexcept
: acVendor {}
, acBrand  {}
, bIsIntel (false)
, bIsAMD   (false)
, i01ECX   (0)
, i01EDX   (0)
, i07EBX   (0)
, i07ECX   (0)
, i81ECX   (0)
, i81EDX   (0)
, iXCR     (0u)
{
    coreList<std::array<coreInt32, 4u>> aaiData;
    coreList<std::array<coreInt32, 4u>> aaiDataEx;
    std::array<coreInt32, 4u> aiPage;

    // get highest valid function IDs
    CORE_CPUID_FUNC(aiPage.data(), CORE_CPUID_BASIC, 0)
    const coreUint32 iNum = aiPage[0];
    CORE_CPUID_FUNC(aiPage.data(), CORE_CPUID_EXTENDED, 0)
    const coreUint32 iNumEx = aiPage[0];

    // reserve some memory
    if(iNum   > CORE_CPUID_BASIC)    aaiData  .reserve(iNum   + 1u - CORE_CPUID_BASIC);
    if(iNumEx > CORE_CPUID_EXTENDED) aaiDataEx.reserve(iNumEx + 1u - CORE_CPUID_EXTENDED);

    // read all available feature bits
    for(coreUint32 i = CORE_CPUID_BASIC; i <= iNum; ++i)
    {
        CORE_CPUID_FUNC(aiPage.data(), i, 0)
        aaiData.push_back(aiPage);
    }
    for(coreUint32 i = CORE_CPUID_EXTENDED; i <= iNumEx; ++i)
    {
        CORE_CPUID_FUNC(aiPage.data(), i, 0)
        aaiDataEx.push_back(aiPage);
    }

    // save processor vendor string
    if(aaiData[0][1])
    {
        std::memcpy(acVendor,                        &aaiData[0][1], sizeof(coreInt32));
        std::memcpy(acVendor + 1u*sizeof(coreInt32), &aaiData[0][3], sizeof(coreInt32));
        std::memcpy(acVendor + 2u*sizeof(coreInt32), &aaiData[0][2], sizeof(coreInt32));
        std::memset(acVendor + 3u*sizeof(coreInt32), 0, 1u);
    }
    else std::strcpy(acVendor, "Unknown");

    // save processor brand string
    if(iNumEx >= (CORE_CPUID_EXTENDED + 4u))
    {
        std::memcpy(acBrand,                     aaiDataEx[2].data(), sizeof(aiPage));
        std::memcpy(acBrand + 1u*sizeof(aiPage), aaiDataEx[3].data(), sizeof(aiPage));
        std::memcpy(acBrand + 2u*sizeof(aiPage), aaiDataEx[4].data(), sizeof(aiPage));
        std::memset(acBrand + 3u*sizeof(aiPage), 0, 1u);
    }
    else std::strcpy(acBrand, "Unknown");

    // check for specific vendors
         if(!std::strcmp(acVendor, "GenuineIntel")) bIsIntel = true;
    else if(!std::strcmp(acVendor, "AuthenticAMD")) bIsAMD   = true;

    // save all relevant feature bits
    if(iNum >= (CORE_CPUID_BASIC + 1u))
    {
        i01ECX = aaiData[1][2];
        i01EDX = aaiData[1][3];
    }
    if(iNum >= (CORE_CPUID_BASIC + 7u))
    {
        i07EBX = aaiData[7][1];
        i07ECX = aaiData[7][2];
    }
    if(iNumEx >= (CORE_CPUID_EXTENDED + 1u))
    {
        i81ECX = aaiDataEx[1][2];
        i81EDX = aaiDataEx[1][3];
    }

    // save extended control register
    if(coreCPUID::OSXSAVE()) CORE_XGETBV_FUNC(&iXCR)
}