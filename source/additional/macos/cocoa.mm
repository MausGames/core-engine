///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#if defined(__APPLE__)

#include <Foundation/Foundation.h>
#include <string>


// ****************************************************************
/* retrieve bundle path for resource files */
const char* coreCocoaPathResource()
{
    @autoreleasepool
    {
        static const std::string s_sPath = [[[NSBundle mainBundle] resourcePath] UTF8String];
        return s_sPath.c_str();
    }
}


// ****************************************************************
/* retrieve bundle path for framework files */
const char* coreCocoaPathFramework()
{
    @autoreleasepool
    {
        static const std::string s_sPath = [[[NSBundle mainBundle] privateFrameworksPath] UTF8String];
        return s_sPath.c_str();
    }
}


// ****************************************************************
/* retrieve system path for application support files (user folder) */
const char* coreCocoaPathApplicationSupport()
{
    @autoreleasepool
    {
        static const std::string s_sPath = [[NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) firstObject] UTF8String];
        return s_sPath.c_str();
    }
}


// ****************************************************************
/* retrieve system path for cache files (user folder) */
const char* coreCocoaPathCaches()
{
    @autoreleasepool
    {
        static const std::string s_sPath = [[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) firstObject] UTF8String];
        return s_sPath.c_str();
    }
}


#endif /* __APPLE__ */