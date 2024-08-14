///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#if defined(__APPLE__)

#include <locale.h>

extern int coreMain(int argc, char** argv);


// ****************************************************************
/* start up the application */
int main(int argc, char** argv)
{
    // enable UTF-8 locale
    setlocale(LC_CTYPE, "UTF-8");

    // run the application
    return coreMain(argc, argv);
}


#endif /* __APPLE__ */
