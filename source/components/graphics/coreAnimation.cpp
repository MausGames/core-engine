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
: coreResource    ()
, m_pTexture      (NULL)
, m_Division      (corePoint2U8(0u, 0u))
, m_piTime        (NULL)
, m_iFrameCount   (0u)
, m_iTotalTime    (0u)
, m_vCurTexSize   (coreVector2(0.0f,0.0f))
, m_vCurTexOffset (coreVector4(0.0f,0.0f,0.0f,0.0f))
, m_iCurFrame     (0u)
, m_fCurLerp      (0.0f)
, m_eLoad         (eLoad)
, m_Sync          ()
{
    // allocate own grid texture
    m_pTexture = Core::Manager::Resource->LoadNew<coreTexture>();
}


// ****************************************************************
/* destructor */
coreAnimation::~coreAnimation()
{
    // unload resource data
    this->Unload();

    // free own grid texture
    Core::Manager::Resource->Free(&m_pTexture);
}


// ****************************************************************
/* load animation resource data */
coreStatus coreAnimation::Load(coreFile* pFile)
{
    // check for sync object status
    const coreStatus eCheck = m_Sync.Check(0u);
    if(eCheck >= CORE_OK) return eCheck;

    WARN_IF(m_piTime)     return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetSize()) return CORE_ERROR_FILE;   // do not load file data

    // decompress file to animation data
    coreAnimationScope pAnim = IMG_LoadAnimationTyped_IO(pFile->CreateReadStream(), true, coreData::StrExtension(pFile->GetPath()));
    WARN_IF(!pAnim || !pAnim->w || !pAnim->h || !pAnim->count)
    {
        Core::Log->Warning("Animation (%s) could not be loaded (SDL: %s)", m_sName.c_str(), SDL_GetError());
        return CORE_INVALID_DATA;
    }

    ASSERT(std::all_of(pAnim->frames, pAnim->frames + pAnim->count, [&](const SDL_Surface* pFrame) {return (pFrame->w == pAnim->w) && (pFrame->h == pAnim->h);}))

    // check components
    coreUint8 iComponents = SDL_BYTESPERPIXEL(pAnim->frames[0]->format);
    if((HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_RG) || HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_R)) && CORE_GL_SUPPORT(ARB_texture_rg))
    {
        WARN_IF(iComponents != 3u) {}
        else iComponents = HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_RG) ? 2u : 1u;
    }

    // check load configuration
    const coreTextureMode eMode = ((!HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_NO_COMPRESS) && coreMath::IsPot(pAnim->w) && coreMath::IsPot(pAnim->h)) ? (HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_COMPRESS_HIGH) ? CORE_TEXTURE_MODE_COMPRESS_HIGH : CORE_TEXTURE_MODE_COMPRESS) : CORE_TEXTURE_MODE_DEFAULT) |
                                  ((!HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_NO_FILTER)) ? CORE_TEXTURE_MODE_FILTER  : CORE_TEXTURE_MODE_DEFAULT) |
                                  ((!HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_NEAREST))   ? CORE_TEXTURE_MODE_DEFAULT : CORE_TEXTURE_MODE_NEAREST);

    if(Core::Config->GetBool(CORE_CONFIG_GRAPHICS_TEXTURECOMPRESSION) && HAS_FLAG(eMode, CORE_TEXTURE_MODE_COMPRESS))
    {
        const coreUint8 iPot = coreMath::CeilPot(pAnim->count);

        // set grid division to permit compression
        m_Division[0] = F_TO_UI(SQRT(I_TO_F(iPot)));
        m_Division[1] = iPot / m_Division[0];
    }
    else
    {
        constexpr coreUint8 aiPreset[] = {0u, 1u, 1u, 1u, 2u, 1u, 2u, 1u, 2u, 3u, 2u, 3u, 3u, 2u, 2u, 3u, 4u, 3u, 3u, 4u, 4u, 3u, 4u, 4u, 4u, 5u, 3u, 3u, 4u, 5u, 5u, 4u, 4u};

        // set grid division to minimize unused space (but do not stretch too much)
        m_Division[0] = (coreUint16(pAnim->count) < ARRAY_SIZE(aiPreset)) ? aiPreset[pAnim->count] : F_TO_UI(SQRT(I_TO_F(pAnim->count)));
        m_Division[1] = coreMath::CeilAlign(pAnim->count, m_Division[0]) / m_Division[0];
    }

    ASSERT((m_Division[0] <= m_Division[1]) && (m_Division[0] * m_Division[1] <= pAnim->count))

    // save properties
    m_iFrameCount = pAnim->count;
    m_vCurTexSize = coreVector2(1.0f / I_TO_F(m_Division[0]), 1.0f / I_TO_F(m_Division[1]));

    // allocate grid data
    const coreUint32 iPackWidth  = pAnim->w * m_Division[0];
    const coreUint32 iPackHeight = pAnim->h * m_Division[1];
    const coreUint32 iPackSize   = iPackWidth * iPackHeight * iComponents;
    coreByte*        pPackData   = ZERO_NEW(coreByte, iPackSize);

    // allocate frame delays
    m_piTime = new coreUint16[m_iFrameCount];

    coreSurfaceScope pScope = NULL;
    for(coreUintW i = 0u, ie = LOOP_NONZERO(m_iFrameCount); i < ie; ++i)
    {
        SDL_Surface* pFrame = pAnim->frames[i];

        // convert to smaller texture format (if supported)
        if(iComponents < 3u) pScope = pFrame = coreTexture::CreateReduction(iComponents, pFrame, pScope);

        ASSERT(!SDL_MUSTLOCK(pFrame))

        const coreUint32 iPitchFrame = pFrame->w * SDL_BYTESPERPIXEL(pFrame->format);
        const coreUint32 iPitchPack  = iPitchFrame * m_Division[0];
        const coreUintW  iOffset     = (i % m_Division[0]) * iPitchFrame + (i / m_Division[0]) * iPitchPack * pFrame->h;

        // copy frame data into grid data
        for(coreUintW j = 0u, je = LOOP_NONZERO(pFrame->h); j < je; ++j)
        {
            std::memcpy(pPackData + (j * iPitchPack) + iOffset, s_cast<coreByte*>(pFrame->pixels) + (j * pFrame->pitch), iPitchFrame);
        }

        // save frame delay
        m_piTime[i]   = pAnim->delays[i];
        m_iTotalTime += pAnim->delays[i];
    }

    // create grid texture
    m_pTexture->Create(iPackWidth, iPackHeight, CORE_TEXTURE_SPEC_COMPONENTS(iComponents), eMode);
    m_pTexture->Modify(0u, 0u, iPackWidth, iPackHeight, iPackSize, pPackData);
    ZERO_DELETE(pPackData)

    // add debug label
    Core::Graphics->LabelOpenGL(GL_TEXTURE, m_pTexture->GetIdentifier(), m_sName.c_str());

    Core::Log->Info("Animation (%s, %.0f x %.0f, %u components, %u levels, %u:%u:%u frames, %.2f seconds, %s) loaded", m_sName.c_str(), m_pTexture->GetResolution().x, m_pTexture->GetResolution().y, m_pTexture->GetSpec().iComponents, m_pTexture->GetLevels(), m_iFrameCount, m_Division[0], m_Division[1], this->GetTotalTime(), m_pTexture->GetCompressed() ? "compressed" : "standard");
    return m_Sync.Create(CORE_SYNC_CREATE_FLUSHED) ? CORE_BUSY : CORE_OK;
}


// ****************************************************************
/* unload animation resource data */
coreStatus coreAnimation::Unload()
{
    if(!m_piTime) return CORE_INVALID_CALL;

    // unload grid texture
    m_pTexture->Unload();

    // delete frame delays
    SAFE_DELETE_ARRAY(m_piTime)
    if(!m_sName.empty()) Core::Log->Info("Animation (%s) unloaded", m_sName.c_str());

    // delete sync object
    m_Sync.Delete();

    // reset properties
    m_Division      = corePoint2U8(0u, 0u);
    m_iFrameCount   = 0u;
    m_iTotalTime    = 0u;
    m_vCurTexSize   = coreVector2(0.0f,0.0f);
    m_vCurTexOffset = coreVector4(0.0f,0.0f,0.0f,0.0f);
    m_iCurFrame     = 0u;
    m_fCurLerp      = 0.0f;

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
    ASSERT((fLerp >= 0.0f) && (fLerp < 1.0f))

    // calculate new texture coordinates
    const coreUint16  iFrameFrom  = (iFrame)      % m_iFrameCount;
    const coreUint16  iFrameTo    = (iFrame + 1u) % m_iFrameCount;
    const coreVector2 vOffsetFrom = coreVector2(I_TO_F(iFrameFrom % m_Division[0]), I_TO_F(iFrameFrom / m_Division[0])) * m_vCurTexSize;
    const coreVector2 vOffsetTo   = coreVector2(I_TO_F(iFrameTo   % m_Division[0]), I_TO_F(iFrameTo   / m_Division[0])) * m_vCurTexSize;

    // set animation values
    m_vCurTexOffset = coreVector4(vOffsetFrom, vOffsetTo);
    m_iCurFrame     = iFrameFrom;
    m_fCurLerp      = fLerp;

    return (iFrame >= m_iFrameCount - 1u);
}


// ****************************************************************
/* change current animation state over time */
void coreAnimation::__ChangeTime(const coreUint16 iTime)
{
    ASSERT(iTime < m_iTotalTime)

    coreUint16 iCurrent = 0u;

    // search for target frame
    for(coreUintW i = 0u, ie = LOOP_NONZERO(m_iFrameCount); i < ie; ++i)
    {
        const coreUint16 iPrev = iCurrent;
        iCurrent += m_piTime[i];

        if(iCurrent > iTime)
        {
            // change current animation state
            this->ChangeFrame(i, STEP(I_TO_F(iPrev), I_TO_F(iCurrent), I_TO_F(iTime)));
            break;
        }
    }
}