///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#if defined(__ANDROID__)

#include <jni.h>
#include <SDL2/SDL_main.h>

extern int coreMain(int argc, char** argv);


// ****************************************************************
/* JNI access objects */
JavaVM* g_pJNIJavaVM   = NULL;   // Java Virtual Machine object
jclass  g_pJNIClass    = NULL;   // reference to the activity class (java.lang.class)
jobject g_pJNIActivity = NULL;   // reference to the activity (extends android.app.activity)

/* called before the application to initialize JNI bindings in the SDL library */
extern void SDL_Android_Init(JNIEnv* pEnv, jclass pCls);


// ****************************************************************
/* start up the application */
void Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv* pEnv, jclass pCls, jobject pObj)
{
    // fetch JNI access objects
    (*pEnv)->GetJavaVM(pEnv, &g_pJNIJavaVM);
    g_pJNIClass    = (jclass)((*pEnv)->NewGlobalRef(pEnv, pCls));
    g_pJNIActivity = (*pEnv)->NewGlobalRef(pEnv, (*pEnv)->GetStaticObjectField(pEnv, g_pJNIClass, (*pEnv)->GetStaticFieldID(pEnv, g_pJNIClass, "mSingleton", "Lorg/libsdl/app/SDLActivity;")));

    // initialize the SDL library
    SDL_Android_Init(pEnv, pCls);
    SDL_SetMainReady();

    // run the application
    char* argv[] = {SDL_strdup("CoreApp"), NULL};
    coreMain(sizeof(argv) / sizeof(argv[0]) - 1, argv);
}


#endif /* __ANDROID__ */
