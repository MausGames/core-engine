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
        const float afDataStrip[] = {-0.5f,  0.5f, 0.0f, 0.001f, 0.001f,
                                     -0.5f, -0.5f, 0.0f, 0.001f, 0.999f,
                                      0.5f,  0.5f, 0.0f, 0.999f, 0.001f,
                                      0.5f, -0.5f, 0.0f, 0.999f, 0.999f};

        // create global model object
        coreObject2D::s_pModel = new coreModel();
        coreObject2D::s_pModel->SetPrimitiveType(GL_TRIANGLE_STRIP);

        // define vertex data
        coreVertexBuffer* pBuffer = coreObject2D::s_pModel->CreateVertexBuffer(4, 5*sizeof(float), afDataStrip, GL_STATIC_DRAW);
        pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 3, GL_FLOAT, 0);
        pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TEXTURE_NUM,  2, GL_FLOAT, 3*sizeof(float));

        // use same model in particle systems
        coreParticleSystem::s_pModel = coreObject2D::s_pModel;

        Core::Log->Info("Global Model Object created");
    }
    else
    {
        // delete global model object
        SAFE_DELETE(coreObject2D::s_pModel);
        coreParticleSystem::s_pModel = NULL;
    }
}