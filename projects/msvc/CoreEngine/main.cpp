#include "Core.h"


std::vector<coreTexturePtr> Ptr;


void CoreApp::Init()
{
}


void CoreApp::Exit()
{
    for(coreUint i = 0; i < Ptr.size(); ++i)
        Ptr[i].SetActive(false);

    Ptr.clear();
}


void CoreApp::Render()
{
    for(coreUint i = 0; i < Ptr.size(); ++i)
    {
        if(Ptr[i].IsActive())
        {
            glLoadIdentity();
            Core::Graphic->EnableOrtho();

            Ptr[i]->Enable(0);
            glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2f(0.0f, 0.0f); glVertex3d(-300.0+50*i, 50.0,-1.0);
                glTexCoord2f(0.0f, 1.0f); glVertex3d(-300.0+50*i,-50.0,-1.0);
                glTexCoord2f(1.0f, 0.0f); glVertex3d(-200.0+50*i, 50.0,-1.0);
                glTexCoord2f(1.0f, 1.0f); glVertex3d(-200.0+50*i,-50.0,-1.0);
            glEnd();
            coreTexture::DisableAll();
        }
    }
}


void CoreApp::Move()
{
    if(Core::Input->GetKeyboardButton(SDL_SCANCODE_A, CORE_INPUT_PRESS))
    {
        std::vector<std::string> sList;
        coreFile::SearchFolder("data/textures", "*.tga", &sList);

        for(coreUint i = 0; i < sList.size(); ++i)
        {
            coreTexturePtr NewPtr = Core::Manager::Resource->Load<coreTexture>(sList[i].c_str());
            Ptr.push_back(NewPtr);
        }
    }

    if(Core::Input->GetKeyboardButton(SDL_SCANCODE_S, CORE_INPUT_PRESS))
        Core::Reset();
}