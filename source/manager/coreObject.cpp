//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreObjectManager::coreObjectManager()noexcept
{
    // create global model objects
    this->__Reset(CORE_RESOURCE_RESET_INIT);

    Core::Log->Info("Object Manager created");
}


// ****************************************************************
// destructor
coreObjectManager::~coreObjectManager()
{
    // delete global model objects
    this->__Reset(CORE_RESOURCE_RESET_EXIT);
    
    Core::Log->Info("Object Manager destroyed");
}


// ****************************************************************
// reset with the resource manager
void coreObjectManager::__Reset(const coreResourceReset& bInit)
{
    if(bInit)
    {
        const float afDataStrip[] = {-0.5f,  0.5f, 0.0f, 0.0f, 0.0f,
                                     -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
                                      0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
                                      0.5f, -0.5f, 0.0f, 1.0f, 1.0f};

        // create global model object
        coreObject2D::s_pModel = new coreModel();
        coreObject2D::s_pModel->SetPrimitiveType(GL_TRIANGLE_STRIP);

        // define vertex data
        coreVertexBuffer* pBuffer = coreObject2D::s_pModel->CreateVertexBuffer(4, 5*sizeof(float), afDataStrip, CORE_DATABUFFER_STORAGE_STATIC);
        pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 3, GL_FLOAT, 0);
        pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TEXCOORD_NUM, 2, GL_FLOAT, 3*sizeof(float));

        // use same model in particle systems
        coreParticleSystem::s_pModel = coreObject2D::s_pModel;

        Core::Log->Info("Global Model Object created");

        if(!CORE_GL_SUPPORT(EXT_framebuffer_blit))
        {
            // create frame buffer fallback
            coreFrameBuffer::s_pBlitFallback = new coreObject2D();

            // load required shaders
            coreFrameBuffer::s_pBlitFallback->DefineProgram(Core::Manager::Resource->Load<coreProgram>("default_2d_program", CORE_RESOURCE_UPDATE_AUTO, NULL))
                ->AttachShader(Core::Manager::Resource->Load<coreShader>("default_2d.vert", CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_2d.vert"))
                ->AttachShader(Core::Manager::Resource->Load<coreShader>("default_2d.frag", CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_2d.frag"))
                ->Finish();

            Core::Log->Warning("Frame Buffer Fallback created");
        }
    }
    else
    {
        // delete global model object
        SAFE_DELETE(coreObject2D::s_pModel);
        coreParticleSystem::s_pModel = NULL;

        // delete frame buffer fallback
        SAFE_DELETE(coreFrameBuffer::s_pBlitFallback)
    }
}