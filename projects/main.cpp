#include "Core.h"


std::vector<coreTexturePtr> g_apTextures;
coreModelPtr g_pModel;


void CoreApp::Init()
{
    g_pModel = Core::Manager::Resource->Load<coreModel>("data/models/default.md5mesh");

    //glDisable(GL_CULL_FACE);
    //glDisable(GL_LIGHTING);
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}


void CoreApp::Exit()
{
    for(coreUint i = 0; i < g_apTextures.size(); ++i)
        g_apTextures[i].SetActive(false);

    g_apTextures.clear();

    g_pModel.SetActive(false);


}


void CoreApp::Render()
{
    for(coreUint i = 0; i < MIN((int)g_apTextures.size(),1); ++i)
    {
        if(g_apTextures[i].IsActive())
        {
            //glMultMatrixf(coreMatrix::Translation(coreVector3(float(i),0.0f,-5.0f)));

            g_apTextures[i]->Enable(0);
            g_pModel->Render();
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
            g_apTextures.push_back(NewPtr);
        }
    }

    if(Core::Input->GetKeyboardButton(SDL_SCANCODE_S, CORE_INPUT_PRESS))
        Core::Reset();

    coreVector3 vCamPos = Core::Graphics->GetCamPosition();
    if(Core::Input->GetKeyboardButton(SDL_SCANCODE_RIGHT, CORE_INPUT_HOLD))
        vCamPos += coreVector3(Core::System->GetTime(),0.0f,0.0f);
    else if(Core::Input->GetKeyboardButton(SDL_SCANCODE_LEFT, CORE_INPUT_HOLD))
        vCamPos -= coreVector3(Core::System->GetTime(),0.0f,0.0f);
    Core::Graphics->SetCamera(&vCamPos, NULL, NULL);
}