// Modified version for Core Engine
// Please use the original file from http://hg.libsdl.org/SDL/file/tip/src/main/android/SDL_android_main.c or http://www.libsdl.org/

#if defined(__ANDROID__)

#include <SDL2/SDL_config.h>
#include <SDL2/SDL_main.h>
#include <jni.h>


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
    char* argv[2] = {SDL_strdup("SDL_app"), NULL};
    SDL_main(1, argv);
}


#endif /* __ANDROID__ */
