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
// define texture through resource file
const coreTexturePtr& coreObject::DefineTextureFile(const coreByte& iUnit, const char* pcPath)
{
    SDL_assert(iUnit < CORE_TEXTURE_UNITS);

    // set and return texture object
    m_apTexture[iUnit] = Core::Manager::Resource->LoadFile<coreTexture>(pcPath);
    return m_apTexture[iUnit];
}


// ****************************************************************
// define texture through linked resource
const coreTexturePtr& coreObject::DefineTextureLink(const coreByte& iUnit, const char* pcName)
{
    SDL_assert(iUnit < CORE_TEXTURE_UNITS);

    // set and return texture object
    m_apTexture[iUnit] = Core::Manager::Resource->LoadLink<coreTexture>(pcName);
    return m_apTexture[iUnit];
}


// ****************************************************************
// define shader-program through shared memory
const coreProgramShr& coreObject::DefineProgramShare(const char* pcName)
{
    // set and return shader-program object
    m_pProgram = Core::Manager::Memory->Share<coreProgram>(pcName);
    return m_pProgram;
}


// ****************************************************************
// constructor
coreObjectManager::coreObjectManager()noexcept
{
    // create global model objeczs
    this->__Reset(true);

    Core::Log->Info("Object Manager created");
}


// ****************************************************************
// destructor
coreObjectManager::~coreObjectManager()
{
    // delete global model objects
    this->__Reset(false);
    
    Core::Log->Info("Object Manager destroyed");
}


// ****************************************************************
// reset with the resource manager
void coreObjectManager::__Reset(const bool& bInit)
{
    if(bInit)
    {
        float afDataStrip[] = {-0.5f,  0.5f, 0.0f, 0.001f, 0.001f,
                               -0.5f, -0.5f, 0.0f, 0.001f, 0.999f,
                                0.5f,  0.5f, 0.0f, 0.999f, 0.001f,
                                0.5f, -0.5f, 0.0f, 0.999f, 0.999f};

        // create global model object
        coreObject2D::s_pModel = new coreModel();
        coreObject2D::s_pModel->SetPrimitiveType(GL_TRIANGLE_STRIP);

        // define vertex data
        coreVertexBuffer* pBuffer = coreObject2D::s_pModel->CreateVertexBuffer(4, sizeof(float)*5, afDataStrip, GL_STATIC_DRAW);
        pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 3, GL_FLOAT, 0);
        pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TEXTURE_NUM,  2, GL_FLOAT, sizeof(float)*3);

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