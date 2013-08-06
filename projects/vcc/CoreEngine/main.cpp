#include "Core.h"


void CoreApp::Init()
{
    coreVector3 test1 = coreVector3(1.0f,1.0f,1.0f);
    coreVector3 test2 = coreVector3(1.0f,1.0f,1.0f);
    coreVector3 test3 = test1+test2;

    coreTexturePtr Ptr = Core::Manager::Resource->Load<coreTexture>("data/textures/default_white.tga");
}


void CoreApp::Exit()
{

}


void CoreApp::Render()
{

}


void CoreApp::Move()
{

}