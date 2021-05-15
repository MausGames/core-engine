///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#if defined(_WIN32)


// ****************************************************************
/* select configuration */
#define WIN32_LEAN_AND_MEAN              // exclude some of the less common features
#define _WIN32_WINNT _WIN32_WINNT_WIN7   // set minimum supported Windows version

#define NOCLIPBOARD
#define NOCOLOR
#define NOCOMM
#define NOCRYPT
// #define NOCTLMGR
#define NODEFERWINDOWPOS
#define NODRAWTEXT
#define NOGDI
#define NOGDICAPMASKS
#define NOHELP
#define NOICONS
#define NOIME
#define NOKANJI
#define NOKERNEL
#define NOKEYSTATES
// #define NOMB
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
// #define NOMSG
// #define NONLS
#define NOOPENFILE
#define NOPROFILER
#define NORASTEROPS
#define NOSCROLL
#define NOSERVICE
// #define NOSHOWWINDOW
#define NOSOUND
#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOTEXTMETRIC
// #define NOUSER
#define NOVIRTUALKEYCODES
#define NOWH
// #define NOWINMESSAGES
#define NOWINOFFSETS
#define NOWINSTYLES


// ****************************************************************
/* include primary header */
#include <Windows.h>


// ****************************************************************
/* remove aliases */
#undef CreateFile
#undef DeleteFile
#undef GetCommandLine
#undef RotateLeft32
#undef RotateLeft64
#undef RotateRight32
#undef RotateRight64


#endif /* _WIN32 */