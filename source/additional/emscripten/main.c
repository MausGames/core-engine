///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#if defined(__EMSCRIPTEN__)

extern int coreMain(int argc, char** argv);


// ****************************************************************
/* start up the application */
int main(int argc, char** argv)
{
    // run the application
    return coreMain(argc, argv);
}


#endif /* __EMSCRIPTEN__ */
