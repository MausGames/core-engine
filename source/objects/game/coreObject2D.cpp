///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreObject2D::coreObject2D()noexcept
: coreObject       ()
, m_vPosition      (coreVector2(0.0f,0.0f))
, m_vSize          (coreVector2(0.0f,0.0f))
, m_vDirection     (coreVector2(0.0f,1.0f))
, m_vCenter        (coreVector2(0.0f,0.0f))
, m_vAlignment     (coreVector2(0.0f,0.0f))
, m_mTransform     (coreMatrix3x2::Identity())
, m_bFocused       (false)
, m_vFocusModifier (coreVector2(1.0f,1.0f))
#if defined(_CORE_MOBILE_)
, m_iFinger        (0u)
#endif
{
    // bind to object manager
    Core::Manager::Object->__BindSprite(this);
}

coreObject2D::coreObject2D(const coreObject2D& c)noexcept
: coreObject       (c)
, m_vPosition      (c.m_vPosition)
, m_vSize          (c.m_vSize)
, m_vDirection     (c.m_vDirection)
, m_vCenter        (c.m_vCenter)
, m_vAlignment     (c.m_vAlignment)
, m_mTransform     (c.m_mTransform)
, m_bFocused       (c.m_bFocused)
, m_vFocusModifier (c.m_vFocusModifier)
#if defined(_CORE_MOBILE_)
, m_iFinger        (c.m_iFinger)
#endif
{
    // bind to object manager
    Core::Manager::Object->__BindSprite(this);
}

coreObject2D::coreObject2D(coreObject2D&& m)noexcept
: coreObject       (std::move(m))
, m_vPosition      (m.m_vPosition)
, m_vSize          (m.m_vSize)
, m_vDirection     (m.m_vDirection)
, m_vCenter        (m.m_vCenter)
, m_vAlignment     (m.m_vAlignment)
, m_mTransform     (m.m_mTransform)
, m_bFocused       (m.m_bFocused)
, m_vFocusModifier (m.m_vFocusModifier)
#if defined(_CORE_MOBILE_)
, m_iFinger        (m.m_iFinger)
#endif
{
    // bind to object manager
    Core::Manager::Object->__BindSprite(this);
}


// ****************************************************************
// destructor
coreObject2D::~coreObject2D()
{
    // unbind from object manager
    Core::Manager::Object->__UnbindSprite(this);
}


// ****************************************************************
// assignment operations
coreObject2D& coreObject2D::operator = (const coreObject2D& c)noexcept
{
    // copy properties
    this->coreObject::operator = (c);
    m_vPosition      = c.m_vPosition;
    m_vSize          = c.m_vSize;
    m_vDirection     = c.m_vDirection;
    m_vCenter        = c.m_vCenter;
    m_vAlignment     = c.m_vAlignment;
    m_mTransform     = c.m_mTransform;
    m_bFocused       = c.m_bFocused;
    m_vFocusModifier = c.m_vFocusModifier;
#if defined(_CORE_MOBILE_)
    m_iFinger        = c.m_iFinger;
#endif

    return *this;
}

coreObject2D& coreObject2D::operator = (coreObject2D&& m)noexcept
{
    // move properties
    this->coreObject::operator = (std::move(m));
    m_vPosition      = m.m_vPosition;
    m_vSize          = m.m_vSize;
    m_vDirection     = m.m_vDirection;
    m_vCenter        = m.m_vCenter;
    m_vAlignment     = m.m_vAlignment;
    m_mTransform     = m.m_mTransform;
    m_bFocused       = m.m_bFocused;
    m_vFocusModifier = m.m_vFocusModifier;
#if defined(_CORE_MOBILE_)
    m_iFinger        = m.m_iFinger;
#endif

    return *this;
}


// ****************************************************************
// undefine the visual appearance
void coreObject2D::Undefine()
{
    // reset all resource and memory pointers
    for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS; ++i) m_apTexture[i] = NULL;
    m_pProgram = NULL;
}


// ****************************************************************
// separately enable all resources for rendering
coreBool coreObject2D::Prepare(const coreProgramPtr& pProgram)
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_RENDER)) return false;

    // enable the shader-program
    ASSERT(pProgram)
    if(!pProgram.IsUsable()) return false;
    if(!pProgram->Enable())  return false;

    // update all object uniforms
    coreProgram* pLocal = pProgram.GetResource();
    pLocal->SendUniform(CORE_SHADER_UNIFORM_2D_SCREENVIEW, coreMatrix3(m_mTransform) * Core::Graphics->GetOrtho().m124(), false);
    pLocal->SendUniform(CORE_SHADER_UNIFORM_COLOR,         m_vColor);
    pLocal->SendUniform(CORE_SHADER_UNIFORM_TEXPARAM,      coreVector4(m_vTexSize, m_vTexOffset));

    // enable all active textures
    coreTexture::EnableAll(m_apTexture);
    return true;
}

coreBool coreObject2D::Prepare()
{
    // enable default shader-program
    return this->Prepare(m_pProgram);
}


// ****************************************************************
// render the 2d-object
void coreObject2D::Render(const coreProgramPtr& pProgram)
{
    // enable all resources
    if(this->Prepare(pProgram))
    {
        // draw the model
        Core::Manager::Object->GetLowQuad()->Enable();
        Core::Manager::Object->GetLowQuad()->DrawArrays();
    }
}

void coreObject2D::Render()
{
    // render with default shader-program (no inheritance)
    this->coreObject2D::Render(m_pProgram);
}


// ****************************************************************
// move the 2d-object
void coreObject2D::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    // check current update status
    if(CONTAINS_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM))
    {
        // calculate resolution-modified transformation parameters
        const coreVector2& vResolution     = Core::System->GetResolution();
        const coreVector2  vScreenPosition = m_vPosition * vResolution.Min() + m_vCenter * vResolution;
        const coreVector2  vScreenSize     = m_vSize     * vResolution.Min();

        // update transformation matrix
        m_mTransform._11 = vScreenSize.x *  m_vDirection.y; m_mTransform._12 = vScreenSize.x * m_vDirection.x;
        m_mTransform._21 = vScreenSize.y * -m_vDirection.x; m_mTransform._22 = vScreenSize.y * m_vDirection.y;
        m_mTransform._31 = vScreenPosition.x;               m_mTransform._32 = vScreenPosition.y;

        // add alignment-offset to position
        m_mTransform._31 += 0.5f * m_vAlignment.x * ABS(m_mTransform._11 + m_mTransform._21);
        m_mTransform._32 += 0.5f * m_vAlignment.y * ABS(m_mTransform._12 + m_mTransform._22);

        // reset the update status
        m_eUpdate = CORE_OBJECT_UPDATE_NOTHING;
    }
}


// ****************************************************************
// interact with the 2d-object
void coreObject2D::Interact()
{
    // get resolution-modified transformation parameters
    const coreVector2 vScreenPosition = coreVector2(    m_mTransform._31,  m_mTransform._32);
    const coreVector2 vScreenSize     = coreVector2(ABS(m_mTransform._11 + m_mTransform._21),
                                                    ABS(m_mTransform._12 + m_mTransform._22)) * (0.5f * m_vFocusModifier);

#if defined(_CORE_MOBILE_)

    // reset interaction status
    m_bFocused = false;
    m_iFinger  = 0u;

    Core::Input->ForEachFinger(CORE_INPUT_HOLD, [&](const coreUintW i)
    {
        // get relative finger position
        const coreVector2 vInput = Core::Input->GetTouchPosition(i) * Core::System->GetResolution() - vScreenPosition;

        // test for intersection
        if((ABS(vInput.x) < vScreenSize.x) &&
           (ABS(vInput.y) < vScreenSize.y))
        {
            m_bFocused = true;
            ADD_BIT(m_iFinger, i)
        }
    });

#else

    // get relative mouse cursor position
    const coreVector2 vInput = Core::Input->GetMousePosition() * Core::System->GetResolution() - vScreenPosition;

    // test for intersection
    m_bFocused = (ABS(vInput.x) < vScreenSize.x) &&
                 (ABS(vInput.y) < vScreenSize.y);

#endif
}


// ****************************************************************
// check for direct input
coreBool coreObject2D::IsClicked(const coreUint8 iButton, const coreInputType eType)const
{
#if defined(_CORE_MOBILE_)

    // check for general intersection status
    if(m_bFocused)
    {
        for(coreUintW i = 0u; i < CORE_INPUT_FINGERS; ++i)
        {
            // check for every finger on the object
            if(CONTAINS_BIT(m_iFinger, i) && Core::Input->GetTouchButton(i, eType))
                return true;
        }
    }
    return false;

#else

    // check for interaction status and mouse button
    return (m_bFocused && Core::Input->GetMouseButton(iButton, eType)) ? true : false;

#endif
}


// ****************************************************************
// render the fullscreen-object
void coreFullscreen::Render(const coreProgramPtr& pProgram)
{
    // enable all resources
    if(this->Prepare(pProgram))
    {
        // draw the model
        Core::Manager::Object->GetLowTriangle()->Enable();
        Core::Manager::Object->GetLowTriangle()->DrawArrays();
    }
}

void coreFullscreen::Render()
{
    // render with default shader-program (no inheritance)
    this->coreFullscreen::Render(m_pProgram);
}