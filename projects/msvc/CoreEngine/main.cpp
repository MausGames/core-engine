#include "Core.h"


std::vector<coreTexturePtr> Ptr;

void CoreApp::Init()
{
    coreVector3 test1 = coreVector3(1.0f,1.0f,1.0f);
    coreVector3 test2 = coreVector3(1.0f,1.0f,1.0f);
    coreVector3 test3 = test1+test2;

    Core::Manager::Resource->Load<coreTexture>("data/textures/ship_1_diff.tga");
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
        if(Ptr[i].GetActive())
        {
            glLoadIdentity();
            Core::Graphic->EnableOrtho();

            Ptr[i]->Enable(0);
            glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2f(0.0f, 1.0f); glVertex3d(-300.0+50*i,-50.0,-1.0);
                glTexCoord2f(0.0f, 0.0f); glVertex3d(-300.0+50*i, 50.0,-1.0);
                glTexCoord2f(1.0f, 1.0f); glVertex3d(-200.0+50*i,-50.0,-1.0);
                glTexCoord2f(1.0f, 0.0f); glVertex3d(-200.0+50*i, 50.0,-1.0);
            glEnd();
            coreTexture::DisableAll();
        }
    }
}


void CoreApp::Move()
{
    if(Core::Input->GetKeyboardButton(SDL_SCANCODE_A, CORE_INPUT_PRESS))
    {
        HANDLE hFolder;
        WIN32_FIND_DATA Files;

        // Suchlauf starten
        hFolder = FindFirstFile(coreUtils::Print("%s/data/textures/*.tga", coreUtils::AppPath()), &Files);
        do
        {
            // Dateigültigkeit ermitteln
            if(Files.cFileName[0] != '.' && strchr(Files.cFileName, '.'))
            {
                coreTexturePtr NewPtr = Core::Manager::Resource->Load<coreTexture>(coreUtils::Print("data/textures/%s", Files.cFileName));
                Ptr.push_back(NewPtr);
            }
        }
        while(FindNextFile(hFolder, &Files));
        FindClose(hFolder);
    }

    if(Core::Input->GetKeyboardButton(SDL_SCANCODE_S, CORE_INPUT_PRESS))
        Core::Reset();
}