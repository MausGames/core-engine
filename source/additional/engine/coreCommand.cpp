///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

#if !defined(_CORE_EMSCRIPTEN_) && !defined(_CORE_SWITCH_)


// ****************************************************************
/* run console commands */
coreStatus Core::__RunCommand()
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

    // return available arguments
    if(coreData::GetCommandLine("help"))
    {
        std::puts(PRINT("%s %s", CoreApp::Settings::Name, CoreApp::Settings::Version));
        std::puts("Options:");
        std::puts("  --user-folder-path <value>   set path for storing user-specific files (needs write permission)");
        std::puts("  --config-override <value>    override config values (e.g. \"Base.AsyncMode:false;System.Width:1024\")");
        std::puts("  --input <value>              input file or directory for some commands");
        std::puts("  --output <value>             output file or directory for some commands");
        std::puts("Commands:");
        std::puts("  --compress                   compress <input> file");
        std::puts("  --decompress                 decompress <input> file");
        std::puts("  --pack                       pack <input> directory into <output> file archive");
        std::puts("  --help                       display available arguments");
        std::puts("  --version                    display simple version string");
        return CORE_OK;
    }

    // return simple version string
    if(coreData::GetCommandLine("version"))
    {
        std::puts(CoreApp::Settings::Version);
        return CORE_OK;
    }

    // compress file
    if(coreData::GetCommandLine("compress"))
    {
        // handle required components
        nSetupFunc();

        // retrieve command options
        const coreChar* pcInput = coreData::GetCommandLine("input");
        if(!pcInput)
        {
            std::puts("<input> parameter missing or invalid");
            return CORE_INVALID_DATA;
        }

        // open input file
        coreFile oFile(pcInput);
        if(!oFile.GetSize())
        {
            std::printf("File (%s) could not be opened\n", pcInput);
            return CORE_ERROR_FILE;
        }

        const coreUint32 iSizeBefore = oFile.GetSize();

        // check if already compressed (useful after library update)
        const coreChar* pcOutput;
        if(coreData::StrRight(pcInput, 1u)[0] == 'z')
        {
            // recompress as strong as possible
            oFile.Decompress();
            oFile.Compress(ZSTD_maxCLevel());

            // write to same location
            pcOutput = pcInput;

            // check for efficacy
            if(oFile.GetSize() >= iSizeBefore)
            {
                std::printf("File (%s) already compressed same or better\n", pcOutput);
                return CORE_OK;
            }
        }
        else
        {
            // compress as strong as possible
            oFile.Compress(ZSTD_maxCLevel());

            // write to new location
            pcOutput = PRINT("%sz", pcInput);
        }

        const coreUint32 iSizeAfter = oFile.GetSize();

        // save output file
        if(oFile.Save(pcOutput) != CORE_OK)
        {
            std::printf("File (%s) could not be saved\n", pcOutput);
            return CORE_ERROR_FILE;
        }

        std::printf("File (%s, %.1f KB -> %.1f KB, %.1f%%) compressed\n", pcInput, I_TO_F(iSizeBefore) / 1024.0f, I_TO_F(iSizeAfter) / 1024.0f, (I_TO_F(iSizeAfter) / I_TO_F(iSizeBefore)) * 100.0f);
        return CORE_OK;
    }

    // decompress file
    if(coreData::GetCommandLine("decompress"))
    {
        // handle required components
        nSetupFunc();

        // retrieve command options
        const coreChar* pcInput = coreData::GetCommandLine("input");
        if(!pcInput)
        {
            std::puts("<input> parameter missing or invalid");
            return CORE_INVALID_DATA;
        }

        // check if even compressed
        if(coreData::StrRight(pcInput, 1u)[0] != 'z')
        {
            std::printf("File (%s) is not compressed\n", pcInput);
            return CORE_ERROR_FILE;
        }

        // open input file
        coreFile oFile(pcInput);
        if(!oFile.GetSize())
        {
            std::printf("File (%s) could not be opened\n", pcInput);
            return CORE_ERROR_FILE;
        }

        const coreUint32 iSizeBefore = oFile.GetSize();

        // decompress into original file
        oFile.Decompress();

        const coreUint32 iSizeAfter = oFile.GetSize();

        // write to new location
        const coreChar* pcOutput = coreData::StrLeft(pcInput, std::strlen(pcInput) - 1u);

        // save output file
        if(oFile.Save(pcOutput) != CORE_OK)
        {
            std::printf("File (%s) could not be saved\n", pcOutput);
            return CORE_ERROR_FILE;
        }

        std::printf("File (%s, %.1f KB -> %.1f KB, %.1f%%) decompressed\n", pcInput, I_TO_F(iSizeBefore) / 1024.0f, I_TO_F(iSizeAfter) / 1024.0f, (I_TO_F(iSizeAfter) / I_TO_F(iSizeBefore)) * 100.0f);
        return CORE_OK;
    }

    // pack directory
    if(coreData::GetCommandLine("pack"))
    {
        // handle required components
        nSetupFunc();

        // retrieve command options
        const coreChar* pcInput  = coreData::GetCommandLine("input");
        const coreChar* pcOutput = coreData::GetCommandLine("output");
        if(!pcInput || !pcOutput)
        {
            std::puts("<input> or <output> parameter missing or invalid");
            return CORE_INVALID_DATA;
        }

        // scan input directory
        coreList<coreString> asFileList;
        if(coreData::DirectoryScanTree(pcInput, "*", &asFileList) != CORE_OK)
        {
            std::printf("Directory (%s) could not be scanned\n", pcInput);
            return CORE_ERROR_FILE;
        }

        // cut off common path
        const coreUintW iLen = std::strlen(pcInput) + 1u;

        // add all files to the archive
        coreArchive oArchive;
        FOR_EACH(it, asFileList)
        {
            oArchive.AddFile(it->c_str(), it->c_str() + iLen);
        }

        // save output file
        if(oArchive.Save(pcOutput) != CORE_OK)
        {
            std::printf("Archive (%s) could not be saved\n", pcOutput);
            return CORE_ERROR_FILE;
        }

        std::printf("Directory (%s, %zu files) packed into archive (%s, %.1f KB)\n", pcInput, asFileList.size(), pcOutput, I_TO_F(coreData::FileSize(pcOutput)) / 1024.0f);
        return CORE_OK;
    }

    // unpack archive (hidden command)
    if(coreData::GetCommandLine("unpack"))
    {
        // handle required components
        nSetupFunc();

        // restrict access
        const coreChar* pcPassword = coreData::GetCommandLine("password");
        if(!pcPassword || (coreHashXXH64(pcPassword) != 0x4E9DE2BADCF9107Fu))
        {
            std::puts("Nur für Verrückte!");
            return CORE_ERROR_SYSTEM;
        }

        // retrieve command options
        const coreChar* pcInput  = coreData::GetCommandLine("input");
        const coreChar* pcOutput = coreData::GetCommandLine("output");
        if(!pcInput || !pcOutput)
        {
            std::puts("<input> or <output> parameter missing or invalid");
            return CORE_INVALID_DATA;
        }

        // open input file
        coreArchive oArchive(pcInput);
        if(!oArchive.GetNumFiles())
        {
            std::printf("Archive (%s) could not be opened\n", pcInput);
            return CORE_ERROR_FILE;
        }

        // create directory hierarchy
        coreData::DirectoryCreate(pcOutput);

        // extract all files from the archive
        for(coreUintW i = 0u, ie = oArchive.GetNumFiles(); i < ie; ++i)
        {
            // write to output directory
            coreFile*       pFile  = oArchive.GetFile(i);
            const coreChar* pcPath = PRINT("%s/%s", pcOutput, pFile->GetPath());

            // save output file
            if(pFile->Save(pcPath) != CORE_OK)
            {
                std::printf("File (%s) could not be saved\n", pcPath);
                return CORE_ERROR_FILE;
            }
        }

        std::printf("Archive (%s, %.1f KB) unpacked into directory (%s, %zu files)\n", pcInput, I_TO_F(coreData::FileSize(pcInput)) / 1024.0f, pcOutput, oArchive.GetNumFiles());
        return CORE_OK;
    }

    // run as thin as possible (hidden command)
    if(coreData::GetCommandLine("headless"))
    {
        // handle required components
        nSetupFunc();

        // quickly init and delete application
        STATIC_NEW   (Application)
        STATIC_DELETE(Application)

        return CORE_OK;
    }

    return CORE_BUSY;
}


#else


// ****************************************************************
/* console commands disabled */
coreStatus Core::__RunCommand() {return CORE_BUSY;}


#endif