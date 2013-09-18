#include "Core.h"


std::vector<coreTexturePtr> g_apTextures;
coreModelPtr g_pModel;
coreProgram* g_pProgram;


void CoreApp::Init()
{
    g_pProgram = NULL;
g_pModel = Core::Manager::Resource->Load<coreModel>("data/models/default.md5mesh");
    glDisable(GL_CULL_FACE);
    //glDisable(GL_LIGHTING);
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}


void CoreApp::Exit()
{
    for(coreUint i = 0; i < g_apTextures.size(); ++i)
        g_apTextures[i].SetActive(false);

    g_apTextures.clear();

    g_pModel.SetActive(false);

    SAFE_DELETE(g_pProgram)
}


void CoreApp::Render()
{
    for(int i = 0; i < MIN((int)g_apTextures.size(),1); ++i)
    {
        if(g_apTextures[i].IsActive())
        {
            //glMultMatrixf(coreMatrix::Translation(coreVector3(float(i),0.0f,-5.0f)));

            g_pProgram->Enable();
            g_apTextures[i]->Enable(0);
            g_pModel->Render();
            coreTexture::DisableAll();
            g_pProgram->Disable();
        }
    }
}


void CoreApp::Move()
{
    if(Core::Input->GetKeyboardButton(SDL_SCANCODE_A, CORE_INPUT_PRESS))
    {
//        std::vector<std::string> sList;
//        coreFile::SearchFolder("data/textures", "*.tga", &sList);
//
//        for(coreUint i = 0; i < sList.size(); ++i)
//        {
//            coreTexturePtr NewPtr = Core::Manager::Resource->Load<coreTexture>(sList[i].c_str());
//            g_apTextures.push_back(NewPtr);
//        }

        g_apTextures.push_back(Core::Manager::Resource->Load<coreTexture>("data/textures/cursor_alph.tga"));



        g_pProgram = new coreProgram();
        g_pProgram->AttachShader("data/shaders/default.vs");
        g_pProgram->AttachShader("data/shaders/default.fs");
        g_pProgram->Link();
    }

    if(Core::Input->GetKeyboardButton(SDL_SCANCODE_S, CORE_INPUT_PRESS))
        Core::Reset();

    coreVector3 vCamPos = Core::Graphics->GetCamPosition();
    vCamPos.z = 10.0f;
    if(Core::Input->GetKeyboardButton(SDL_SCANCODE_D, CORE_INPUT_HOLD))
        vCamPos += coreVector3(Core::System->GetTime(),0.0f,0.0f);
    else if(Core::Input->GetKeyboardButton(SDL_SCANCODE_F, CORE_INPUT_HOLD))
        vCamPos -= coreVector3(Core::System->GetTime(),0.0f,0.0f);
    Core::Graphics->SetCamera(&vCamPos, NULL, NULL);
}