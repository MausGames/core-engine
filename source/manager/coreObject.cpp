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
    float afDataStrip[] = {-0.5f,  0.5f, 0.0f, 0.001f, 0.001f,
                           -0.5f, -0.5f, 0.0f, 0.001f, 0.999f,
                            0.5f,  0.5f, 0.0f, 0.999f, 0.001f,
                            0.5f, -0.5f, 0.0f, 0.999f, 0.999f};

    // create 2d model object
    coreObject2D::s_pModel = new coreModel();
    coreObject2D::s_pModel->SetPrimitiveType(GL_TRIANGLE_STRIP);

    // define 2d vertex data
    coreVertexBuffer* pBuffer = coreObject2D::s_pModel->CreateVertexBuffer(4, sizeof(float)*5, afDataStrip, GL_STATIC_DRAW);
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 3, GL_FLOAT, 0);
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TEXTURE_NUM,  2, GL_FLOAT, sizeof(float)*3);

    Core::Log->Info("Created 2D Model Object");

    // use point particles with geometry shader support
    if(GLEW_VERSION_3_2)
    {
        float afDataPoint[] = {0.0f, 0.0f, 0.0f};

        // create particle model object
        coreParticleSystem::s_pModel = new coreModel();
        coreParticleSystem::s_pModel->SetPrimitiveType(GL_POINT);

        // define particle vertex data
        pBuffer = coreParticleSystem::s_pModel->CreateVertexBuffer(1, sizeof(float)*3, afDataPoint, GL_STATIC_DRAW);
        pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 3, GL_FLOAT, 0);

        Core::Log->Info("Created Particle Model Object");
    }
    else coreParticleSystem::s_pModel = coreObject2D::s_pModel;

    Core::Log->Info("Object Manager created");
}


// ****************************************************************
// destructor
coreObjectManager::~coreObjectManager()
{
    // delete standard model objects
    if(coreObject2D::s_pModel != coreParticleSystem::s_pModel)
        SAFE_DELETE(coreObject2D::s_pModel);
    SAFE_DELETE(coreParticleSystem::s_pModel)
    
    Core::Log->Info("Object Manager destroyed");
}