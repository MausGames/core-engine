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
/* constructor */
coreObjectManager::coreObjectManager()noexcept
: m_pBlitFallback (NULL)
{
    // allocate low-memory model object
    m_pLowModel = Core::Manager::Resource->LoadNew<coreModel>();

    // start up the object manager
    this->__Reset(CORE_RESOURCE_RESET_INIT);

    Core::Log->Info(CORE_LOG_BOLD("Object Manager created"));
}


// ****************************************************************
/* destructor */
coreObjectManager::~coreObjectManager()
{
    // shut down the object manager
    this->__Reset(CORE_RESOURCE_RESET_EXIT);

    // free low-memory model object
    Core::Manager::Resource->Free(&m_pLowModel);

    Core::Log->Info(CORE_LOG_BOLD("Object Manager destroyed"));
}


// ****************************************************************
/* test collision between two 3d-objects */
coreBool FUNC_NOALIAS coreObjectManager::TestCollision(const coreObject3D* pObject1, const coreObject3D* pObject2)
{
    ASSERT(pObject1 && pObject2)

    // get collision radius
    const coreFloat fTotalRadius = pObject1->GetCollisionRadius() + pObject2->GetCollisionRadius();

    // calculate distance between both objects
    const coreVector3 vDiff = pObject1->GetPosition() - pObject2->GetPosition();

    // check for sphere intersection
    if(vDiff.LengthSq() > fTotalRadius * fTotalRadius) return false;

    // get collision range and rotation
    const coreVector3& vRange1    = pObject1->GetCollisionRange();
    const coreVector3& vRange2    = pObject2->GetCollisionRange();
    const coreVector4& vRotation1 = pObject1->GetRotation();
    const coreVector4& vRotation2 = pObject2->GetRotation();

    // revert second rotation
    const coreVector4 vRevRotation2 = vRotation2.QuatConjugate();

    // calculate relative transformation matrix (with absolute values to check only for maximums)
    const coreMatrix3 M = coreMatrix3::Quat(coreVector4::QuatMultiply(vRevRotation2, vRotation1));
    const coreMatrix3 S = coreMatrix3(ABS(M._11), ABS(M._12), ABS(M._13),
                                      ABS(M._21), ABS(M._22), ABS(M._23),
                                      ABS(M._31), ABS(M._32), ABS(M._33));

    // rotate and move first object relative to second (but distance only with single rotation)
    const coreVector3 D1 = vRevRotation2.QuatApply(vDiff);
    const coreVector3 R1 = vRange1 * S;

    // check for first boundary intersection
    if(ABS(D1.x) > R1.x + vRange2.x) return false;
    if(ABS(D1.y) > R1.y + vRange2.y) return false;
    if(ABS(D1.z) > R1.z + vRange2.z) return false;

    // revert first rotation
    const coreVector4 vRevRotation1 = vRotation1.QuatConjugate();

    // rotate and move second object relative to first
    const coreVector3 D2 = vRevRotation1.QuatApply(vDiff);
    const coreVector3 R2 = vRange2 * S.Transposed();

    // check for second boundary intersection
    if(ABS(D2.x) > vRange1.x + R2.x) return false;
    if(ABS(D2.y) > vRange1.y + R2.y) return false;
    if(ABS(D2.z) > vRange1.z + R2.z) return false;

    return true;
}


// ****************************************************************
/* test collision between 3d-object and line */
coreFloat FUNC_NOALIAS coreObjectManager::TestCollision(const coreObject3D* pObject, const coreVector3& vLinePos, const coreVector3& vLineDir)
{
    ASSERT(pObject && vLineDir.IsNormalized())

    // get collision radius
    const coreFloat& fRadius = pObject->GetCollisionRadius();

    // calculate distance between both objects
    const coreVector3 vDiff = pObject->GetPosition() - vLinePos;

    // calculate range parameters
    const coreFloat fAdjacent   = coreVector3::Dot(vDiff, vLineDir);
    const coreFloat fOppositeSq = vDiff.LengthSq() - fAdjacent * fAdjacent;
    const coreFloat fRadiusSq   = fRadius * fRadius;

    // check for sphere intersection (return distance from line position to intersection point on success)
    return (fOppositeSq <= fRadiusSq) ? (fAdjacent - SQRT(fRadiusSq - fOppositeSq)) : 0.0f;
}


// ****************************************************************
/* reset with the resource manager */
void coreObjectManager::__Reset(const coreResourceReset& bInit)
{
    if(bInit)
    {
        const coreUint32 aiDataStrip[4] = {coreVector2(-0.5f, 0.5f).PackSnorm2x16(),
                                           coreVector2(-0.5f,-0.5f).PackSnorm2x16(),
                                           coreVector2( 0.5f, 0.5f).PackSnorm2x16(),
                                           coreVector2( 0.5f,-0.5f).PackSnorm2x16()};

        // create low-memory model object
        m_pLowModel->SetBoundingRange (coreVector3(0.5f,0.5f,0.0f));
        m_pLowModel->SetBoundingRadius(0.7071f);
        m_pLowModel->SetPrimitiveType (GL_TRIANGLE_STRIP);

        // define vertex data
        coreVertexBuffer* pBuffer = m_pLowModel->CreateVertexBuffer(4u, sizeof(coreUint32), aiDataStrip, CORE_DATABUFFER_STORAGE_STATIC);
        pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 2u, GL_SHORT, false, 0u);

        Core::Log->Info("Low-memory model object created");

        if(!CORE_GL_SUPPORT(EXT_framebuffer_blit))
        {
            // create frame buffer fallback
            m_pBlitFallback = new coreObject2D();
            m_pBlitFallback->DefineProgram("default_2d_program");

            Core::Log->Warning("Frame buffer fallback created");
        }

        // force update of all existing 2d-objects
        FOR_EACH(it, m_apSpriteList)
        {
            (*it)->m_iUpdate = CORE_OBJECT_UPDATE_ALL;
            (*it)->coreObject2D::Move();
            (*it)->m_iUpdate = CORE_OBJECT_UPDATE_ALL;
        }
    }
    else
    {
        // unload low-memory model object
        m_pLowModel->Unload();

        // delete frame buffer fallback
        SAFE_DELETE(m_pBlitFallback)
    }
}


// ****************************************************************
/* update all objects and collisions */
void coreObjectManager::__UpdateObjects()
{
    // loop through all objects
    FOR_EACH(it, m_aapObjectList)
    {
        FOR_EACH_DYN(et, *it)
        {
            // check for invalid pointers and remove them
            if(*et) DYN_KEEP  (et)
               else DYN_REMOVE(et, *it)
        }
    }

    // loop through all collisions
    const coreUint32& iCurFrame = Core::System->GetCurFrame();
    FOR_EACH_DYN(it, m_aObjectCollision)
    {
        // check for old entries and remove them
        if(it->iLastFrame == iCurFrame) DYN_KEEP  (it)
                                   else DYN_REMOVE(it, m_aObjectCollision)
    }
}


// ****************************************************************
/* bind 3d-object to type */
void coreObjectManager::__BindObject(coreObject3D* pObject, const coreInt32& iType)
{
    ASSERT(pObject && iType)

#if defined(_CORE_DEBUG_)

        // check for duplicate objects
        FOR_EACH(it, m_aapObjectList)
            FOR_EACH(et, *it)
                ASSERT(*et != pObject)

#endif

    // add object to requested list
    m_aapObjectList[iType].push_back(pObject);
}


// ****************************************************************
/* unbind 3d-object from type */
void coreObjectManager::__UnbindObject(coreObject3D* pObject, const coreInt32& iType)
{
    ASSERT(pObject && iType)

    // get requested list
    coreObjectList& oList = m_aapObjectList[iType];

    // loop through all objects
    FOR_EACH(it, oList)
    {
        if(*it == pObject)
        {
            // invalidate pointer and delete later
            *it = NULL;
            return;
        }
    }
}


// ****************************************************************
/* handle and track new collisions */
coreBool coreObjectManager::__NewCollision(const coreObject3D* pObject1, const coreObject3D* pObject2)
{
    // loop through all collisions
    FOR_EACH(it, m_aObjectCollision)
    {
        if((it->pObject1 == pObject1) && (it->pObject2 == pObject2))
        {
            // update frame number
            it->iLastFrame = Core::System->GetCurFrame();
            return false;
        }
    }

    // create new collision
    coreObjectCollision oNewCollision;
    oNewCollision.pObject1   = pObject1;
    oNewCollision.pObject2   = pObject2;
    oNewCollision.iLastFrame = Core::System->GetCurFrame();

    // add collision to list
    m_aObjectCollision.push_back(oNewCollision);
    return true;
}