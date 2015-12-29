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
coreObject2D::coreObject2D()noexcept
: coreObject       ()
, m_vPosition      (coreVector2(0.0f,0.0f))
, m_vSize          (coreVector2(0.0f,0.0f))
, m_vDirection     (coreVector2(0.0f,1.0f))
, m_vCenter        (coreVector2(0.0f,0.0f))
, m_vAlignment     (coreVector2(0.0f,0.0f))
, m_mTransform     (coreMatrix3::Identity())
, m_bFocused       (false)
, m_vFocusModifier (coreVector2(1.0f,1.0f))
#if defined(_CORE_ANDROID_)
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
#if defined(_CORE_ANDROID_)
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
#if defined(_CORE_ANDROID_)
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
    // copy remaining properties
    coreObject::operator = (c);
    m_vPosition      = c.m_vPosition;
    m_vSize          = c.m_vSize;
    m_vDirection     = c.m_vDirection;
    m_vCenter        = c.m_vCenter;
    m_vAlignment     = c.m_vAlignment;
    m_mTransform     = c.m_mTransform;
    m_bFocused       = c.m_bFocused;
    m_vFocusModifier = c.m_vFocusModifier;
#if defined(_CORE_ANDROID_)
    m_iFinger        = c.m_iFinger;
#endif

    return *this;
}

coreObject2D& coreObject2D::operator = (coreObject2D&& m)noexcept
{
    // move remaining properties
    coreObject::operator = (std::move(m));
    m_vPosition      = m.m_vPosition;
    m_vSize          = m.m_vSize;
    m_vDirection     = m.m_vDirection;
    m_vCenter        = m.m_vCenter;
    m_vAlignment     = m.m_vAlignment;
    m_mTransform     = m.m_mTransform;
    m_bFocused       = m.m_bFocused;
    m_vFocusModifier = m.m_vFocusModifier;
#if defined(_CORE_ANDROID_)
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
// render the 2d-object
void coreObject2D::Render(const coreProgramPtr& pProgram)
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_RENDER)) return;

    // enable the shader-program
    if(!pProgram.IsUsable()) return;
    if(!pProgram->Enable())  return;

    // update all object uniforms
    pProgram->SendUniform(CORE_SHADER_UNIFORM_2D_SCREENVIEW, m_mTransform * Core::Graphics->GetOrtho().m124(), false);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_COLOR,         m_vColor);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_TEXPARAM,      coreVector4(m_vTexSize, m_vTexOffset));

    // enable all active textures
    for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS; ++i)
        if(m_apTexture[i].IsUsable()) m_apTexture[i]->Enable(i);

    // draw the model
    Core::Manager::Object->GetLowModel()->Enable();
    Core::Manager::Object->GetLowModel()->DrawArrays();
}

void coreObject2D::Render()
{
    // render with default shader-program
    coreObject2D::Render(m_pProgram);
}


// ****************************************************************
// move the 2d-object
void coreObject2D::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    // check current update status
    if(CONTAINS_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_TRANSFORM))
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
        m_iUpdate = CORE_OBJECT_UPDATE_NOTHING;
    }
}


// ****************************************************************
// interact with the 2d-object
void coreObject2D::Interact()
{
    // get resolution-modified transformation parameters
    const coreVector2 vScreenPosition = coreVector2(    m_mTransform._31,      m_mTransform._32);
    const coreVector2 vScreenSize     = coreVector2(ABS(m_mTransform._11), ABS(m_mTransform._22)) * 0.5f * m_vFocusModifier;

#if defined(_CORE_ANDROID_)

    // reset interaction status
    m_bFocused = false;
    m_iFinger  = 0u;

    Core::Input->ForEachFinger(CORE_INPUT_HOLD, [&](const coreUintW i)
    {
        // get relative finger position
        const coreVector2 vInput = Core::Input->GetTouchPosition(i) * Core::System->GetResolution() - vScreenPosition;

        // test for intersection
        if(ABS(vInput.x) < vScreenSize.x &&
           ABS(vInput.y) < vScreenSize.y)
        {
            m_bFocused = true;
            ADD_BIT(m_iFinger, i)
        }
    });

#else

    // get relative mouse cursor position
    const coreVector2 vInput = Core::Input->GetMousePosition() * Core::System->GetResolution() - vScreenPosition;

    // test for intersection
    m_bFocused = (ABS(vInput.x) < vScreenSize.x &&
                  ABS(vInput.y) < vScreenSize.y);

#endif
}


// ****************************************************************
// check for direct input
coreBool coreObject2D::IsClicked(const coreUint8 iButton, const coreInputType iType)const
{
#if defined(_CORE_ANDROID_)

    // check for general intersection status
    if(m_bFocused)
    {
        for(coreUintW i = 0u; i < CORE_INPUT_FINGERS; ++i)
        {
            // check for every finger on the object
            if(CONTAINS_BIT(m_iFinger, i) && Core::Input->GetTouchButton(i, iType))
                return true;
        }
    }
    return false;

#else

    // check for interaction status and mouse button
    return (m_bFocused && Core::Input->GetMouseButton(iButton, iType)) ? true : false;

#endif
}