#include "Core.h"
/* Warning: currently a sandbox-file, do not use it */


std::vector<coreTexturePtr> g_apTextures;
coreModelPtr g_pModel;
std::shared_ptr<coreProgram> g_pProgram1;
std::shared_ptr<coreProgram> g_pProgram2;

coreMusicPlayer* g_pMusic;


void CoreApp::Init()
{
    g_pProgram1 = NULL;

    g_pModel = Core::Manager::Resource->Load<coreModel>("data/models/default.md5mesh");

    g_pMusic = new coreMusicPlayer();
    g_pMusic->AddFile("data/feel.ogg");
    g_pMusic->AddFile("data/korn.ogg");
    g_pMusic->SetFade(2.0f);
    g_pMusic->Control()->Play();

    //glDisable(GL_LIGHTING);
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}


void CoreApp::Exit()
{
    for(coreUint i = 0; i < g_apTextures.size(); ++i)
        g_apTextures[i].SetActive(false);

    g_apTextures.clear();

    g_pModel.SetActive(false);

    g_pProgram1.reset();
    g_pProgram2.reset();

    SAFE_DELETE(g_pMusic)
}


void CoreApp::Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(int i = 0; i < MIN((int)g_apTextures.size(),1); ++i)
    {
        if(g_apTextures[i].IsActive())
        {
            //glMultMatrixf(coreMatrix::Translation(coreVector3(float(i),0.0f,-5.0f)));

            g_pProgram2->Enable();
            g_apTextures[i]->Enable(0);
            g_pModel->Render();
            coreTexture::DisableAll();
            g_pProgram2->Disable();
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

        g_apTextures.push_back(Core::Manager::Resource->Load<coreTexture>("data/textures/android.png"));



        g_pProgram1 = Core::Manager::Memory->Share<coreProgram>("test_ptr");
        g_pProgram1->AttachShader("data/shaders/default.vs")
                   ->AttachShader("data/shaders/default.fs")
                   ->Link();

        g_pProgram2 = Core::Manager::Memory->Share<coreProgram>("test_ptr");
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

    g_pMusic->Update();
    if(Core::Input->GetKeyboardButton(SDL_SCANCODE_Q, CORE_INPUT_PRESS))
    {
        g_pMusic->Next();
        g_pMusic->Control()->SeekTime(10.0);
    }
}