///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

#if defined(_CORE_DEBUG_)

#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_USE_STD_HEADERS
#define DOCTEST_CONFIG_NO_COMPARISON_WARNING_SUPPRESSION
#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_NO_MULTITHREADING

#include <doctest.h>

extern "C" coreInt32              g_iArgc;
extern "C" const coreChar* const* g_ppcArgv;


// ****************************************************************
/* unit tests */
DOCTEST_TEST_CASE("Unit Tests")
{
    DOCTEST_SUBCASE("coreData")
    {
        DOCTEST_SUBCASE("Compress/Decompress")
        {
            const coreByte acData[] = "trans rights are human rights";

            coreByte*  pCompress     = NULL;
            coreUint32 iCompressSize = 0u;
            DOCTEST_CHECK(coreData::Compress(acData, sizeof(acData), &pCompress, &iCompressSize) == CORE_OK);

            coreByte*  pDecompress     = NULL;
            coreUint32 iDecompressSize = 0u;
            DOCTEST_CHECK(coreData::Decompress(pCompress, iCompressSize, &pDecompress, &iDecompressSize) == CORE_OK);

            DOCTEST_CHECK(iDecompressSize == sizeof(acData));
            DOCTEST_CHECK(!std::memcmp(pDecompress, "trans rights are human rights", iDecompressSize));

            SAFE_DELETE_ARRAY(pCompress)
            SAFE_DELETE_ARRAY(pDecompress)
        }

        DOCTEST_SUBCASE("Scramble/Unscramble")
        {
            coreByte acData[] = "be gay, do crime";

            coreData::Scramble(acData, sizeof(acData));
            DOCTEST_CHECK(!std::memcmp(acData, "\x9e\x98\x2e\x72\xa5\x2f\x19\xe3\x21\xa2\x5a\xdd\xf9\xae\x96\x3c\x4d", sizeof(acData)));

            coreData::Unscramble(acData, sizeof(acData));
            DOCTEST_CHECK(!std::memcmp(acData, "be gay, do crime", sizeof(acData)));
        }

        DOCTEST_SUBCASE("StrVersion")
        {
            DOCTEST_CHECK(coreData::StrVersion("21.1.7")     == corePoint3U8(21u, 1u,  7u));
            DOCTEST_CHECK(coreData::StrVersion("ABC 3.64.A") == corePoint3U8(3u,  64u, 0u));
            DOCTEST_CHECK(coreData::StrVersion(".19-2.43")   == corePoint3U8(19u, 0u,  0u));
            DOCTEST_CHECK(coreData::StrVersion("A4BC.")      == corePoint3U8(4u,  0u,  0u));
            DOCTEST_CHECK(coreData::StrVersion("ABC")        == corePoint3U8(0u,  0u,  0u));
            DOCTEST_CHECK(coreData::StrVersion("")           == corePoint3U8(0u,  0u,  0u));
        }
    }

    DOCTEST_SUBCASE("coreMath")
    {
        DOCTEST_SUBCASE("DivUp")
        {
            DOCTEST_CHECK(coreMath::DivUp(7, 4) == 2);
            DOCTEST_CHECK(coreMath::DivUp(4, 4) == 1);
            DOCTEST_CHECK(coreMath::DivUp(1, 4) == 1);
            DOCTEST_CHECK(coreMath::DivUp(0, 4) == 0);
        }
    }
}


// ****************************************************************
/* run unit tests */
coreStatus Core::__RunTest()
{
    const auto nSetupFunc = []()
    {
        // init SDL library (minimum)
        WARN_IF(!SDL_Init(0u)) {}

        // init log file
        STATIC_NEW(Log, coreData::UserFolderShared("log.html"))

        // automatically shut down everything on exit
        WARN_IF(std::atexit([]() {STATIC_DELETE(Log) SDL_Quit();})) {}
    };

    // only execute on demand (based on CLion plugin)
    if(coreData::GetCommandLine("r"))
    {
        // handle required components
        nSetupFunc();

        // run requested unit tests and exit
        doctest::Context oContext(g_iArgc, g_ppcArgv);
        std::exit(oContext.run());
    }

    return CORE_BUSY;
}


#else


// ****************************************************************
/* unit tests disabled */
coreStatus Core::__RunTest() {return CORE_BUSY;}


#endif