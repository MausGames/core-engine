///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

#if defined(_CORE_DEBUG_) && !defined(_CORE_EMSCRIPTEN_) && !defined(_CORE_SWITCH_)

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

        DOCTEST_SUBCASE("StrToUpper")
        {
            DOCTEST_CHECK_EQ(coreData::StrToUpper("Straße 123"), "STRAßE 123");
        }

        DOCTEST_SUBCASE("StrToLower")
        {
            DOCTEST_CHECK_EQ(coreData::StrToLower("Straße 123"), "straße 123");
        }

        DOCTEST_SUBCASE("StrIsUpper")
        {
            DOCTEST_CHECK_EQ(coreData::StrIsUpper("Straße 123"), false);
            DOCTEST_CHECK_EQ(coreData::StrIsUpper("STRAßE 123"), true);
        }

        DOCTEST_SUBCASE("StrIsLower")
        {
            DOCTEST_CHECK_EQ(coreData::StrIsLower("Straße 123"), false);
            DOCTEST_CHECK_EQ(coreData::StrIsLower("straße 123"), true);
        }

        DOCTEST_SUBCASE("StrToUpperUTF8")
        {
            DOCTEST_CHECK_EQ(coreData::StrToUpperUTF8("Straße 123"), "STRASSE 123");

            DOCTEST_CHECK_EQ(coreData::StrToUpperUTF8("Shadow Quality"), "SHADOW QUALITY");
            DOCTEST_CHECK_EQ(coreData::StrToUpperUTF8("Qualité des ombres"), "QUALITÉ DES OMBRES");
            DOCTEST_CHECK_EQ(coreData::StrToUpperUTF8("Calidad de las sombras"), "CALIDAD DE LAS SOMBRAS");
            DOCTEST_CHECK_EQ(coreData::StrToUpperUTF8("Qualità delle ombre"), "QUALITÀ DELLE OMBRE");
            DOCTEST_CHECK_EQ(coreData::StrToUpperUTF8("Schatten Qualität"), "SCHATTEN QUALITÄT");
            DOCTEST_CHECK_EQ(coreData::StrToUpperUTF8("Качество теней"), "КАЧЕСТВО ТЕНЕЙ");
            DOCTEST_CHECK_EQ(coreData::StrToUpperUTF8("阴影质量"), "阴影质量");
            DOCTEST_CHECK_EQ(coreData::StrToUpperUTF8("陰影品質"), "陰影品質");
            DOCTEST_CHECK_EQ(coreData::StrToUpperUTF8("シャドウ品質"), "シャドウ品質");
            DOCTEST_CHECK_EQ(coreData::StrToUpperUTF8("그림자 품질"), "그림자 품질");
        }

        DOCTEST_SUBCASE("StrToLowerUTF8")
        {
            DOCTEST_CHECK_EQ(coreData::StrToLowerUTF8("Straße 123"), "straße 123");

            DOCTEST_CHECK_EQ(coreData::StrToLowerUTF8("Shadow Quality"), "shadow quality");
            DOCTEST_CHECK_EQ(coreData::StrToLowerUTF8("Qualité des ombres"), "qualité des ombres");
            DOCTEST_CHECK_EQ(coreData::StrToLowerUTF8("Calidad de las sombras"), "calidad de las sombras");
            DOCTEST_CHECK_EQ(coreData::StrToLowerUTF8("Qualità delle ombre"), "qualità delle ombre");
            DOCTEST_CHECK_EQ(coreData::StrToLowerUTF8("Schatten Qualität"), "schatten qualität");
            DOCTEST_CHECK_EQ(coreData::StrToLowerUTF8("Качество теней"), "качество теней");
            DOCTEST_CHECK_EQ(coreData::StrToLowerUTF8("阴影质量"), "阴影质量");
            DOCTEST_CHECK_EQ(coreData::StrToLowerUTF8("陰影品質"), "陰影品質");
            DOCTEST_CHECK_EQ(coreData::StrToLowerUTF8("シャドウ品質"), "シャドウ品質");
            DOCTEST_CHECK_EQ(coreData::StrToLowerUTF8("그림자 품질"), "그림자 품질");
        }

        DOCTEST_SUBCASE("StrIsUpperUTF8")
        {
            DOCTEST_CHECK_EQ(coreData::StrIsUpperUTF8("Straße 123"), false);
            DOCTEST_CHECK_EQ(coreData::StrIsUpperUTF8("STRAßE 123"), false);

            DOCTEST_CHECK_EQ(coreData::StrIsUpperUTF8("SHADOW QUALITY"), true);
            DOCTEST_CHECK_EQ(coreData::StrIsUpperUTF8("Qualité des ombres"), false);
            DOCTEST_CHECK_EQ(coreData::StrIsUpperUTF8("calidad de las sombras"), false);
            DOCTEST_CHECK_EQ(coreData::StrIsUpperUTF8("QUALITÀ DELLE OMBRE"), true);
            DOCTEST_CHECK_EQ(coreData::StrIsUpperUTF8("Schatten Qualität"), false);
            DOCTEST_CHECK_EQ(coreData::StrIsUpperUTF8("качество теней"), false);
            DOCTEST_CHECK_EQ(coreData::StrIsUpperUTF8("阴影质量"), true);
            DOCTEST_CHECK_EQ(coreData::StrIsUpperUTF8("陰影品質"), true);
            DOCTEST_CHECK_EQ(coreData::StrIsUpperUTF8("シャドウ品質"), true);
            DOCTEST_CHECK_EQ(coreData::StrIsUpperUTF8("그림자 품질"), true);
        }

        DOCTEST_SUBCASE("StrIsLowerUTF8")
        {
            DOCTEST_CHECK_EQ(coreData::StrIsLowerUTF8("Straße 123"), false);
            DOCTEST_CHECK_EQ(coreData::StrIsLowerUTF8("straße 123"), true);

            DOCTEST_CHECK_EQ(coreData::StrIsLowerUTF8("SHADOW QUALITY"), false);
            DOCTEST_CHECK_EQ(coreData::StrIsLowerUTF8("Qualité des ombres"), false);
            DOCTEST_CHECK_EQ(coreData::StrIsLowerUTF8("calidad de las sombras"), true);
            DOCTEST_CHECK_EQ(coreData::StrIsLowerUTF8("QUALITÀ DELLE OMBRE"), false);
            DOCTEST_CHECK_EQ(coreData::StrIsLowerUTF8("Schatten Qualität"), false);
            DOCTEST_CHECK_EQ(coreData::StrIsLowerUTF8("качество теней"), true);
            DOCTEST_CHECK_EQ(coreData::StrIsLowerUTF8("阴影质量"), true);
            DOCTEST_CHECK_EQ(coreData::StrIsLowerUTF8("陰影品質"), true);
            DOCTEST_CHECK_EQ(coreData::StrIsLowerUTF8("シャドウ品質"), true);
            DOCTEST_CHECK_EQ(coreData::StrIsLowerUTF8("그림자 품질"), true);
        }

        DOCTEST_SUBCASE("StrVersion")
        {
            DOCTEST_CHECK_EQ(coreData::StrVersion("21.1.7"),     corePoint3U8(21u, 1u,  7u));
            DOCTEST_CHECK_EQ(coreData::StrVersion("ABC 3.64.A"), corePoint3U8(3u,  64u, 0u));
            DOCTEST_CHECK_EQ(coreData::StrVersion(".19-2.43"),   corePoint3U8(19u, 0u,  0u));
            DOCTEST_CHECK_EQ(coreData::StrVersion("A4BC."),      corePoint3U8(4u,  0u,  0u));
            DOCTEST_CHECK_EQ(coreData::StrVersion("ABC"),        corePoint3U8(0u,  0u,  0u));
            DOCTEST_CHECK_EQ(coreData::StrVersion(""),           corePoint3U8(0u,  0u,  0u));
        }
    }

    DOCTEST_SUBCASE("coreMath")
    {
        DOCTEST_SUBCASE("DivUp")
        {
            DOCTEST_CHECK_EQ(coreMath::DivUp(7, 4), 2);
            DOCTEST_CHECK_EQ(coreMath::DivUp(4, 4), 1);
            DOCTEST_CHECK_EQ(coreMath::DivUp(1, 4), 1);
            DOCTEST_CHECK_EQ(coreMath::DivUp(0, 4), 0);
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