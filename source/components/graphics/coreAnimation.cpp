///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
coreAnimation::coreAnimation(const coreTextureLoad eLoad)noexcept
: m_pTextureFrom (NULL)
, m_pTextureTo   (NULL)
, m_ppFrame      (NULL)
, m_piTime       (NULL)
, m_iFrameCount  (0u)
, m_iTotalTime   (0u)
, m_fLerp        (0.0f)
, m_eLoad        (eLoad)
, m_Sync         ()
{
    // create own proxy textures
    m_pTextureFrom = Core::Manager::Resource->LoadProxyNew();
    m_pTextureTo   = Core::Manager::Resource->LoadProxyNew();
}


// ****************************************************************
/* destructor */
coreAnimation::~coreAnimation()
{
    // unload resource data
    this->Unload();

    // free own proxy textures
    Core::Manager::Resource->Free(&m_pTextureFrom);
    Core::Manager::Resource->Free(&m_pTextureTo);
}


// ****************************************************************
/* load animation resource data */
coreStatus coreAnimation::Load(coreFile* pFile)
{
    // check for sync object status
    const coreStatus eCheck = m_Sync.Check(0u);
    if(eCheck >= CORE_OK) return eCheck;

    WARN_IF(m_ppFrame)    return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetSize()) return CORE_ERROR_FILE;   // do not load file data

    // decompress file to animation data
    IMG_Animation* pAnim = IMG_LoadAnimationTyped_RW(pFile->CreateReadStream(), 1, coreData::StrExtension(pFile->GetPath()));
    if(!pAnim)
    {
        Core::Log->Warning("Animation (%s) could not be loaded (SDL: %s)", m_sName.c_str(), SDL_GetError());
        return CORE_INVALID_DATA;
    }

    // allocate frame memory
    m_iFrameCount = pAnim->count;
    m_ppFrame     = new coreTexturePtr[m_iFrameCount];
    m_piTime      = new coreUint16    [m_iFrameCount];
    ASSERT(m_iFrameCount)

    coreSurfaceScope pScope = NULL;

    for(coreUintW i = 0u, ie = m_iFrameCount; i < ie; ++i)
    {
        SDL_Surface* pData = pAnim->frames[i];

        // convert to smaller texture format (if supported)
        if((HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_RG) || HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_R)) && CORE_GL_SUPPORT(ARB_texture_rg))
        {
            WARN_IF(pData->format->BytesPerPixel != 3u) {}
            else pScope = pData = coreTexture::CreateReduction(HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_RG) ? 2u : 1u, pData, pScope);
        }

        ASSERT(!SDL_MUSTLOCK(pData))

        // calculate data size
        const coreUint8  iComponents = pData->format->BytesPerPixel;
        const coreUint32 iDataSize   = pData->w * pData->h * iComponents;
        ASSERT(iComponents && iDataSize)

        // check load configuration
        const coreTextureMode eMode = ((!HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_NO_COMPRESS) && coreMath::IsPot(pData->w) && coreMath::IsPot(pData->h)) ? CORE_TEXTURE_MODE_COMPRESS : CORE_TEXTURE_MODE_DEFAULT) |
                                      ((!HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_NO_FILTER))                                                             ? CORE_TEXTURE_MODE_FILTER   : CORE_TEXTURE_MODE_DEFAULT) |
                                      ((!HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_NEAREST))                                                               ? CORE_TEXTURE_MODE_DEFAULT  : CORE_TEXTURE_MODE_NEAREST);

        // allocate own frame texture
        m_ppFrame[i] = Core::Manager::Resource->LoadNew<coreTexture>();

        // create frame texture
        m_ppFrame[i]->Create(pData->w, pData->h, CORE_TEXTURE_SPEC_COMPONENTS(iComponents), eMode);
        m_ppFrame[i]->Modify(0u, 0u, pData->w, pData->h, iDataSize, s_cast<coreByte*>(pData->pixels));

        // add debug label
        Core::Graphics->LabelOpenGL(GL_TEXTURE, m_ppFrame[i]->GetIdentifier(), m_sName.c_str());

        // save frame delay
        m_piTime[i]   = pAnim->delays[i];
        m_iTotalTime += pAnim->delays[i];
    }

    // delete animation data
    IMG_FreeAnimation(pAnim);

    // init proxy textures
    const coreTexturePtr& pFirst = m_ppFrame[0];
    Core::Manager::Resource->AssignProxy(m_pTextureFrom.GetHandle(), pFirst.GetHandle());
    Core::Manager::Resource->AssignProxy(m_pTextureTo  .GetHandle(), pFirst.GetHandle());

    Core::Log->Info("Animation (%s, %.0f x %.0f, %u components, %u levels, %u frames, %.2f seconds, %s) loaded", m_sName.c_str(), pFirst->GetResolution().x, pFirst->GetResolution().y, pFirst->GetSpec().iComponents, pFirst->GetLevels(), m_iFrameCount, this->GetTotalTime(), pFirst->GetCompressed() ? "compressed" : "standard");
    return m_Sync.Create(CORE_SYNC_CREATE_FLUSHED) ? CORE_BUSY : CORE_OK;
}


// ****************************************************************
/* unload animation resource data */
coreStatus coreAnimation::Unload()
{
    if(!m_ppFrame) return CORE_INVALID_CALL;

    // reset proxy textures
    Core::Manager::Resource->AssignProxy(m_pTextureFrom.GetHandle(), NULL);
    Core::Manager::Resource->AssignProxy(m_pTextureTo  .GetHandle(), NULL);

    // free own frame textures
    for(coreUintW i = 0u, ie = m_iFrameCount; i < ie; ++i)
    {
        Core::Manager::Resource->Free(&m_ppFrame[i]);
    }

    // free frame memory
    SAFE_DELETE_ARRAY(m_ppFrame)
    SAFE_DELETE_ARRAY(m_piTime)
    if(!m_sName.empty()) Core::Log->Info("Animation (%s) unloaded", m_sName.c_str());

    // delete sync object
    m_Sync.Delete();

    // reset properties
    m_iFrameCount = 0u;
    m_iTotalTime  = 0u;
    m_fLerp       = 0.0f;

    return CORE_OK;
}


// ****************************************************************
/* change current animation state over time */
void coreAnimation::ChangeTimeLoop(const coreFloat fTime)
{
    // loop infinitely
    this->__ChangeTime(F_TO_UI(fTime * 1000.0f) % m_iTotalTime);
}

coreBool coreAnimation::ChangeTimeOnce(const coreFloat fTime)
{
    // limit to the last frame
    const coreUint32 iTime  = F_TO_UI(fTime * 1000.0f);
    const coreUint32 iLimit = m_iTotalTime - m_piTime[m_iFrameCount - 1u];

    // run only once
    this->__ChangeTime(MIN(iTime, iLimit));
    return (iTime >= iLimit);
}


// ****************************************************************
/* change current animation state directly */
coreBool coreAnimation::ChangeFrame(const coreUint16 iFrame, const coreFloat fLerp)
{
    // set interpolation value
    ASSERT((fLerp >= 0.0f) && (fLerp < 1.0f))
    m_fLerp = fLerp;

    // set proxy textures
    Core::Manager::Resource->AssignProxy(m_pTextureFrom.GetHandle(), m_ppFrame[(iFrame)      % m_iFrameCount].GetHandle());
    Core::Manager::Resource->AssignProxy(m_pTextureTo  .GetHandle(), m_ppFrame[(iFrame + 1u) % m_iFrameCount].GetHandle());
    return (iFrame >= m_iFrameCount - 1u);
}


// ****************************************************************
/* change current animation state over time */
void coreAnimation::__ChangeTime(const coreUint16 iTime)
{
    ASSERT(iTime < m_iTotalTime)

    coreUint16 iCurrent = 0u;

    for(coreUintW i = 0u, ie = m_iFrameCount; i < ie; ++i)
    {
        const coreUint16 iPrev = iCurrent;
        iCurrent += m_piTime[i];

        if(iCurrent > iTime)
        {
            // set interpolation value
            m_fLerp = STEP(I_TO_F(iPrev), I_TO_F(iCurrent), I_TO_F(iTime));

            // set proxy textures
            Core::Manager::Resource->AssignProxy(m_pTextureFrom.GetHandle(), m_ppFrame[(i)]                     .GetHandle());
            Core::Manager::Resource->AssignProxy(m_pTextureTo  .GetHandle(), m_ppFrame[(i + 1u) % m_iFrameCount].GetHandle());
            break;
        }
    }
}