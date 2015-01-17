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
    // shut down the resource manager
    this->__Reset(CORE_RESOURCE_RESET_EXIT);

    // free low-memory model object
    Core::Manager::Resource->Free(&m_pLowModel);

    Core::Log->Info("Object Manager destroyed");
}


// ****************************************************************
/* test collision between two 3d-objects */
bool coreObjectManager::TestCollision(const coreObject3D* pObject1, const coreObject3D* pObject2)
{
    ASSERT(pObject1 && pObject2)

    // get collision radius
    const float fTotalRadius = pObject1->GetCollisionRadius() + pObject2->GetCollisionRadius();

    // calculate distance between both objects
    const coreVector3 vDiff = pObject1->GetPosition() - pObject2->GetPosition();

    // check for sphere intersection
    if(vDiff.LengthSq() > fTotalRadius * fTotalRadius) return false;

    // get collision range and rotation properties
    const coreVector3 vTotalRange      =  pObject1->GetCollisionRange() + pObject2->GetCollisionRange();
    const coreVector3 avDirection  [2] = {pObject1->GetDirection(),       pObject2->GetDirection()};
    const coreVector3 avOrientation[2] = {pObject1->GetOrientation(),     pObject2->GetOrientation()};

    // project range and distance onto every object axis
    for(int i = 0; i < 2; ++i)
    {
        const coreVector3& D = avDirection[i];

        // check for Y axis intersection
        if(ABS(vDiff      .x * D.x) + ABS(vDiff      .y * D.y) + ABS(vDiff      .z * D.z) >
           ABS(vTotalRange.x * D.x) + ABS(vTotalRange.y * D.y) + ABS(vTotalRange.z * D.z))
            return false;

        const coreVector3 T = coreVector3::Cross(D, avOrientation[i]);

        // check for X axis intersection
        if(ABS(vDiff      .x * T.x) + ABS(vDiff      .y * T.y) + ABS(vDiff      .z * T.z) >
           ABS(vTotalRange.x * T.x) + ABS(vTotalRange.y * T.y) + ABS(vTotalRange.z * T.z))
            return false;

        const coreVector3 B = coreVector3::Cross(D, T);

        // check for Z axis intersection
        if(ABS(vDiff      .x * B.x) + ABS(vDiff      .y * B.y) + ABS(vDiff      .z * B.z) >
           ABS(vTotalRange.x * B.x) + ABS(vTotalRange.y * B.y) + ABS(vTotalRange.z * B.z))
            return false;
    }

    return true;
}


// ****************************************************************
/* test collision between 3d-object and line */
float coreObjectManager::TestCollision(const coreObject3D* pObject, const coreVector3& vLinePos, const coreVector3& vLineDir)
{
    ASSERT(pObject && vLineDir.IsNormalized())

    // get collision radius
    const float& fRadius = pObject->GetCollisionRadius();

    // calculate distance between both objects
    const coreVector3 vDiff = pObject->GetPosition() - vLinePos;

    // calculate range parameters
    const float fAdjacent   = coreVector3::Dot(vDiff, vLineDir);
    const float fOppositeSq = vDiff.LengthSq() - fAdjacent * fAdjacent;
    const float fRadiusSq   = fRadius * fRadius;

    // check for sphere intersection (return distance from line position to intersection point on success)
    return (fOppositeSq <= fRadiusSq) ? (fAdjacent - SQRT(fRadiusSq - fOppositeSq)) : 0.0f;
}


// ****************************************************************
/* reset with the resource manager */
void coreObjectManager::__Reset(const coreResourceReset& bInit)
{
    if(bInit)
    {
        const coreUint aiDataStrip[4] = {coreVector2(-0.5f, 0.5f).PackSnorm2x16(),
                                         coreVector2(-0.5f,-0.5f).PackSnorm2x16(),
                                         coreVector2( 0.5f, 0.5f).PackSnorm2x16(),
                                         coreVector2( 0.5f,-0.5f).PackSnorm2x16()};

        // create low-memory model object
        m_pLowModel->SetBoundingRadius(0.7071f);
        m_pLowModel->SetBoundingRange (coreVector3(0.5f,0.5f,0.0f));
        m_pLowModel->SetPrimitiveType (GL_TRIANGLE_STRIP);

        // define vertex data
        coreVertexBuffer* pBuffer = m_pLowModel->CreateVertexBuffer(4, sizeof(coreUint), aiDataStrip, CORE_DATABUFFER_STORAGE_STATIC);
        pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 2, GL_SHORT, 0);

        Core::Log->Info("Low-memory model object created");

        if(!CORE_GL_SUPPORT(EXT_framebuffer_blit))
        {
            // create frame buffer fallback
            m_pBlitFallback = new coreObject2D();
            m_pBlitFallback->DefineProgram("default_2d_program");

            Core::Log->Warning("Frame buffer fallback created");
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
        FOR_EACH_DYN(et, it->second)
        {
            // check for invalid pointers and remove them
            if(*et) DYN_KEEP  (et)
               else DYN_REMOVE(et, it->second)
        }
    }

    // loop through all collisions
    const coreUint& iCurFrame = Core::System->GetCurFrame();
    FOR_EACH_DYN(it, m_aObjectCollision)
    {
        // check for old entries and remove them
        if(it->iLastFrame == iCurFrame) DYN_KEEP  (it)
                                   else DYN_REMOVE(it, m_aObjectCollision)
    }
}


// ****************************************************************
/* bind 3d-object to type */
void coreObjectManager::__BindObject(coreObject3D* pObject, const int& iType)
{
    ASSERT(pObject && iType)

#if defined(_CORE_DEBUG_)

        // check for duplicate objects
        FOR_EACH(it, m_aapObjectList)
            FOR_EACH(et, it->second)
                ASSERT(*et != pObject)

#endif

    // add object to requested list (may create list)
    m_aapObjectList[iType].push_back(pObject);
}


// ****************************************************************
/* unbind 3d-object from type */
void coreObjectManager::__UnbindObject(coreObject3D* pObject, const int& iType)
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
bool coreObjectManager::__NewCollision(const coreObject3D* pObject1, const coreObject3D* pObject2)
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