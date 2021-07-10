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
/* constructor */
coreObject::coreObject()noexcept
: m_apTexture  {}
, m_pProgram   (NULL)
, m_vColor     (coreVector4(1.0f,1.0f,1.0f,1.0f))
, m_vTexSize   (coreVector2(1.0f,1.0f))
, m_vTexOffset (coreVector2(0.0f,0.0f))
, m_eUpdate    (CORE_OBJECT_UPDATE_ALL)
, m_eEnabled   (CORE_OBJECT_ENABLE_ALL)
, m_iStatus    (0)
{
}


// ****************************************************************
/* constructor */
coreObjectManager::coreObjectManager()noexcept
: m_aapObjectList     {}
, m_aiObjectCollision {}
, m_aiIndex           {}
, m_bIndexDirty       (false)
, m_pLowQuad          (NULL)
, m_pLowTriangle      (NULL)
, m_pBlitFallback     (NULL)
, m_apSpriteList      {}
, m_vSpriteViewDir    (coreVector2(0.0f,1.0f))
, m_vSpriteAltCenter  (coreVector2(0.0f,0.0f))
{
    // allocate low-memory models
    m_pLowQuad     = Core::Manager::Resource->LoadNew<coreModel>();
    m_pLowTriangle = Core::Manager::Resource->LoadNew<coreModel>();

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

    // free low-memory models
    Core::Manager::Resource->Free(&m_pLowQuad);
    Core::Manager::Resource->Free(&m_pLowTriangle);

    Core::Log->Info(CORE_LOG_BOLD("Object Manager destroyed"));
}


// ****************************************************************
/* test collision between two 3d-objects */
coreBool coreObjectManager::TestCollision(const coreObject3D* pObject1, const coreObject3D* pObject2, coreVector3* OUTPUT pvIntersection)
{
    ASSERT(pObject1 && pObject2 && pvIntersection)

    // calculate difference between both objects
    const coreVector3 vDiff = pObject1->GetPosition() - pObject2->GetPosition();

    // get collision radius
    const coreFloat fTotalRadius = pObject1->GetCollisionRadius() + pObject2->GetCollisionRadius();

    // check for sphere intersection
    if((vDiff.LengthSq() > POW2(fTotalRadius)) || coreMath::IsNear(fTotalRadius, 0.0f))
        return false;

    // get collision range and rotation
    const coreVector3 vRange1    = pObject1->GetCollisionRange();
    const coreVector3 vRange2    = pObject2->GetCollisionRange();
    const coreVector4 vRotation1 = pObject1->GetRotation();
    const coreVector4 vRotation2 = pObject2->GetRotation();

    // revert second rotation
    const coreVector4 vRevRotation2 = vRotation2.QuatConjugate();

    // calculate relative transformation matrix (with absolute values to check only for maximums)
    const coreVector4 Q = coreVector4::QuatMultiply(vRevRotation2, vRotation1);
    const coreMatrix3 M = coreMatrix3::FromQuat(Q);
    const coreMatrix3 S = M.Processed(ABS);

    // rotate and move first object relative to second (but difference only with single rotation)
    const coreVector3 D1 = vRevRotation2.QuatApply(vDiff);
    const coreVector3 R1 = vRange1 * S;

    // check for first boundary intersection
    if(ABS(D1.x) > (R1.x + vRange2.x)) return false;
    if(ABS(D1.y) > (R1.y + vRange2.y)) return false;
    if(ABS(D1.z) > (R1.z + vRange2.z)) return false;

    // revert first rotation
    const coreVector4 vRevRotation1 = vRotation1.QuatConjugate();

    // rotate and move second object relative to first
    const coreVector3 D2 = vRevRotation1.QuatApply(-vDiff);
    const coreVector3 R2 = vRange2 * S.Transposed();

    // check for second boundary intersection
    if(ABS(D2.x) > (vRange1.x + R2.x)) return false;
    if(ABS(D2.y) > (vRange1.y + R2.y)) return false;
    if(ABS(D2.z) > (vRange1.z + R2.z)) return false;

    // check if clusters are available for precise collision detection
    const coreBool bPrecise1 = pObject1->GetModel().IsUsable() && pObject1->GetModel()->GetNumClusters();
    const coreBool bPrecise2 = pObject2->GetModel().IsUsable() && pObject2->GetModel()->GetNumClusters();

    // return intersection between two simple models
    if(!bPrecise1 && !bPrecise2)
    {
        (*pvIntersection) = pObject2->GetPosition() + vDiff * (pObject2->GetCollisionRadius() * RCP(fTotalRadius));
        return true;
    }

    // move less complex object to the first position (to improve re-use of transformed data when iterating through clusters of both models)
    const coreBool bSwap = (!bPrecise2 || (bPrecise1 && pObject1->GetModel()->GetNumIndices() > pObject2->GetModel()->GetNumIndices()));
    if(bSwap) std::swap(pObject1, pObject2);

    // prepare relative transformation (only first object will be transformed)
    const coreVector3 vRelPosition = bSwap ? D2                : D1;
    const coreVector4 vRelRotation = bSwap ? Q.QuatConjugate() : Q;

    // get models and object sizes (precise collision detection does not use size-modifiers)
    const coreModel*  pModel1   = pObject1->GetModel().IsUsable() ? pObject1->GetModel().GetResource() : NULL;
    const coreModel*  pModel2   = pObject2->GetModel().GetResource();
    const coreVector3 vSize1    = pObject1->GetSize();
    const coreVector3 vSize2    = pObject2->GetSize();
    const coreFloat   vSizeMax1 = vSize1.Max();
    const coreFloat   vSizeMax2 = vSize2.Max();

    // calculate collision between precise and simple model
    if(!bPrecise1 || !bPrecise2)
    {
        const coreVector3 vPosition1 = vRelPosition;
        const coreFloat   fRadius1   = pObject1->GetCollisionRadius();

        for(coreUintW m = 0u, me = pModel2->GetNumClusters(); m < me; ++m)
        {
            const coreVector3 vPosition2 = vSize2 * pModel2->GetClusterPosition(m);
            const coreFloat   fRadius2   = pModel2->GetClusterRadius(m) * vSizeMax2;

            const coreVector3 vClusterDiff   = vPosition1 - vPosition2;
            const coreFloat   fClusterRadius = fRadius1 + fRadius2;

            if(vClusterDiff.LengthSq() > POW2(fClusterRadius))
                continue;

            (*pvIntersection) = pObject2->GetPosition() + pObject2->GetRotation().QuatApply(vPosition2 + vClusterDiff * (fRadius2 * RCP(fClusterRadius)));
            return true;
        }

        return false;
    }

    // calculate collision between two precise models (Moeller97b)
    for(coreUintW k = 0u, ke = pModel1->GetNumClusters(); k < ke; ++k)
    {
        const coreVector3 vPosition1 = vRelPosition + vRelRotation.QuatApply(vSize1 * pModel1->GetClusterPosition(k));
        const coreFloat   fRadius1   = pModel1->GetClusterRadius(k) * vSizeMax1;

        for(coreUintW m = 0u, me = pModel2->GetNumClusters(); m < me; ++m)
        {
            const coreVector3 vPosition2 = vSize2 * pModel2->GetClusterPosition(m);
            const coreFloat   fRadius2   = pModel2->GetClusterRadius(m) * vSizeMax2;
            const coreFloat   fRadius2Sq = POW2(fRadius2);

            const coreVector3 vClusterDiff   = vPosition1 - vPosition2;
            const coreFloat   fClusterRadius = fRadius1 + fRadius2;

            if(vClusterDiff.LengthSq() > POW2(fClusterRadius))
                continue;

            const coreVector3* pvVertexPosition1 = pModel1->GetVertexPosition();
            const coreVector3* pvVertexPosition2 = pModel2->GetVertexPosition();
            const coreUint16*  piClusterIndex1   = pModel1->GetClusterIndex(k);
            const coreUint16*  piClusterIndex2   = pModel2->GetClusterIndex(m);

            for(coreUintW i = 0u, ie = pModel1->GetClusterNumIndices(k); i < ie; i += 3u)
            {
                coreVector3 A1 = vRelPosition + vRelRotation.QuatApply(vSize1 * pvVertexPosition1[piClusterIndex1[i]]);
                coreVector3 A2 = vRelPosition + vRelRotation.QuatApply(vSize1 * pvVertexPosition1[piClusterIndex1[i+1u]]);
                coreVector3 A3 = vRelPosition + vRelRotation.QuatApply(vSize1 * pvVertexPosition1[piClusterIndex1[i+2u]]);

                const coreVector3 vCross1 = coreVector3::Cross(A2 - A1, A3 - A1);

                if(POW2(coreVector3::Dot(A1 - vPosition2, vCross1)) > fRadius2Sq * vCross1.LengthSq())
                    continue;

                for(coreUintW j = 0u, je = pModel2->GetClusterNumIndices(m); j < je; j += 3u)
                {
                    coreVector3 B1 = vSize2 * pvVertexPosition2[piClusterIndex2[j]];
                    coreVector3 B2 = vSize2 * pvVertexPosition2[piClusterIndex2[j+1u]];
                    coreVector3 B3 = vSize2 * pvVertexPosition2[piClusterIndex2[j+2u]];

                    coreUint32 F1 = (coreVector3::Dot(B1 - A1, vCross1) >= 0.0f) ? 1u : 0u;
                    coreUint32 F2 = (coreVector3::Dot(B2 - A1, vCross1) >= 0.0f) ? 1u : 0u;
                    coreUint32 F3 = (coreVector3::Dot(B3 - A1, vCross1) >= 0.0f) ? 1u : 0u;

                    if((F1 == F2) && (F1 == F3))
                        continue;

                    const coreVector3 vCross2 = coreVector3::Cross(B2 - B1, B3 - B1);

                    coreUint32 G1 = (coreVector3::Dot(A1 - B1, vCross2) >= 0.0f) ? 1u : 0u;
                    coreUint32 G2 = (coreVector3::Dot(A2 - B1, vCross2) >= 0.0f) ? 1u : 0u;
                    coreUint32 G3 = (coreVector3::Dot(A3 - B1, vCross2) >= 0.0f) ? 1u : 0u;

                    if((G1 == G2) && (G1 == G3))
                        continue;

                    if(F2 != F3)
                    {
                        if(F1 != F2)
                        {
                            std::swap(B1, B2);
                            std::swap(F1, F2);
                        }
                        else if(F1 != F3)
                        {
                            std::swap(B1, B3);
                            std::swap(F1, F3);
                        }
                    }

                    if(G2 != G3)
                    {
                        if(G1 != G2)
                        {
                            std::swap(A1, A2);
                            std::swap(G1, G2);
                        }
                        else if(G1 != G3)
                        {
                            std::swap(A1, A3);
                            std::swap(G1, G3);
                        }
                    }

                    if(F1 > 0u) std::swap(A2, A3);
                    if(G1 > 0u) std::swap(B2, B3);

                    if((coreVector3::Dot(A2 - B1, coreVector3::Cross(B2 - B1, A1 - B1)) <= 0.0f) &&
                       (coreVector3::Dot(A1 - B1, coreVector3::Cross(B3 - B1, A3 - B1)) <= 0.0f))
                    {
                        (*pvIntersection) = pObject2->GetPosition() + pObject2->GetRotation().QuatApply((A1 + A2 + A3 + B1 + B2 + B3) * (1.0f/6.0f));
                        return true;
                    }
                }
            }
        }
    }

    return false;
}


// ****************************************************************
/* test collision between 3d-object and ray */
coreBool coreObjectManager::TestCollision(const coreObject3D* pObject, const coreVector3 vRayPos, const coreVector3 vRayDir, coreFloat* OUTPUT pfHitDistance, coreUint8* OUTPUT piHitCount)
{
    ASSERT(pObject && vRayDir.IsNormalized() && pfHitDistance && piHitCount)
    ASSERT(((*piHitCount) >= 1u) && ((*piHitCount) <= CORE_OBJECT_RAY_HITCOUNT))

    // calculate difference between ray and object
    const coreVector3 vDiff = pObject->GetPosition() - vRayPos;

    // get closest point on ray
    const coreFloat fAdjacent = coreVector3::Dot(vDiff, vRayDir);
    const coreFloat fDiffSq   = vDiff.LengthSq();
    const coreFloat fRadiusSq = POW2(pObject->GetCollisionRadius());

    // check if ray moves away from object
    if((fAdjacent < 0.0f) && (fDiffSq > fRadiusSq))
        return false;

    // get minimum distance
    const coreFloat fOppositeSq = fDiffSq - POW2(fAdjacent);

    // check for sphere intersection
    if(fOppositeSq > fRadiusSq)
        return false;

    // check if clusters are available for precise collision detection
    const coreBool bPrecise = pObject->GetModel().IsUsable() && pObject->GetModel()->GetNumClusters();

    // return intersection with simple model
    if(!bPrecise)
    {
        // get half penetration length
        const coreFloat fHalfLen = SQRT(fRadiusSq - fOppositeSq);

        if(fDiffSq > fRadiusSq)
        {
            // ray is outside
            if((*piHitCount) >= 1u) pfHitDistance[0] = fAdjacent - fHalfLen;
            if((*piHitCount) >= 2u) pfHitDistance[1] = fAdjacent + fHalfLen;
            (*piHitCount) = 2u;
        }
        else
        {
            // ray is inside
            if((*piHitCount) >= 1u) pfHitDistance[0] = fAdjacent + fHalfLen;
            (*piHitCount) = 1u;
        }

        return true;
    }

    // transform ray instead of whole object
    const coreVector4 vRevRotation = pObject->GetRotation().QuatConjugate();
    const coreVector3 vRelRayPos   = vRevRotation.QuatApply(-vDiff);
    const coreVector3 vRelRayDir   = vRevRotation.QuatApply(vRayDir);

    // get model and object size (precise collision detection does not use size-modifiers)
    const coreModel*  pModel   = pObject->GetModel().GetResource();
    const coreVector3 vSize    = pObject->GetSize();
    const coreFloat   vSizeMax = vSize.Max();

    // calculate collision with precise model (MoellerTrumbore97)
    if(bPrecise)
    {
        const auto nFilterFunc = [](coreFloat* OUTPUT pfArray, const coreUint8 iCount)
        {
            // sort ascending and remove duplicates
            std::sort(pfArray, pfArray + iCount);
            return std::unique(pfArray, pfArray + iCount, [](const coreFloat A, const coreFloat B) {return coreMath::IsNear(A, B);}) - pfArray;
        };

        coreFloat afHitDistance[CORE_OBJECT_RAY_HITCOUNT + 1u] = {};
        coreUint8 iHitCount                                    = 0u;

        for(coreUintW m = 0u, me = pModel->GetNumClusters(); m < me; ++m)
        {
            const coreVector3 vClusterPos  = vSize * pModel->GetClusterPosition(m);
            const coreVector3 vClusterDiff = vClusterPos - vRelRayPos;

            const coreFloat fAdjacent2 = coreVector3::Dot(vClusterDiff, vRelRayDir);
            const coreFloat fDiffSq2   = vClusterDiff.LengthSq();
            const coreFloat fRadiusSq2 = POW2(pModel->GetClusterRadius(m) * vSizeMax);

            if((fAdjacent2 < 0.0f) && (fDiffSq2 > fRadiusSq2))
                continue;

            const coreFloat fOppositeSq2 = fDiffSq2 - POW2(fAdjacent2);

            if(fOppositeSq2 > fRadiusSq2)
                continue;

            const coreVector3* pvVertexPosition = pModel->GetVertexPosition();
            const coreUint16*  piClusterIndex   = pModel->GetClusterIndex(m);

            for(coreUintW j = 0u, je = pModel->GetClusterNumIndices(m); j < je; j += 3u)
            {
                const coreVector3 V1 = vSize * pvVertexPosition[piClusterIndex[j]];
                const coreVector3 V2 = vSize * pvVertexPosition[piClusterIndex[j+1u]];
                const coreVector3 V3 = vSize * pvVertexPosition[piClusterIndex[j+2u]];

                const coreVector3 W1 = V2 - V1;
                const coreVector3 W2 = V3 - V1;

                const coreVector3 A = coreVector3::Cross(vRelRayDir, W2);
                const coreFloat   B = coreVector3::Dot(W1, A);

                if(coreMath::IsNear(B, 0.0f))
                    continue;

                const coreFloat   C = RCP(B);
                const coreVector3 D = vRelRayPos - V1;
                const coreFloat   E = coreVector3::Dot(D, A) * C;

                if((E < 0.0f) || (E > 1.0f))
                    continue;

                const coreVector3 F = coreVector3::Cross(D, W1);
                const coreFloat   G = coreVector3::Dot(vRelRayDir, F) * C;
                const coreFloat   H = coreVector3::Dot(W2, F) * C;

                if((G < 0.0f) || (G + E > 1.0f) || (H < 0.0f))
                    continue;

                afHitDistance[iHitCount++] = H;

                if(iHitCount == CORE_OBJECT_RAY_HITCOUNT + 1u)
                {
                    // remove duplicates early or drop biggest distance value
                    iHitCount = MIN(nFilterFunc(afHitDistance, CORE_OBJECT_RAY_HITCOUNT + 1u), CORE_OBJECT_RAY_HITCOUNT);
                }
            }
        }

        if(iHitCount)
        {
            // finalize distance values
            const coreUint8 iFinalCount = nFilterFunc(afHitDistance, iHitCount);

            // return collision data
            std::memcpy(pfHitDistance, afHitDistance, sizeof(coreFloat) * MIN(iFinalCount, (*piHitCount)));
            std::memcpy(piHitCount,    &iFinalCount,  sizeof(coreUint8));

            return true;
        }
    }

    return false;
}


// ****************************************************************
/* refresh all existing 2d-objects */
void coreObjectManager::RefreshSprites()
{
    FOR_EACH(it, m_apSpriteList) (*it)->m_eUpdate = CORE_OBJECT_UPDATE_ALL;
}


// ****************************************************************
/* reset with the resource manager */
void coreObjectManager::__Reset(const coreResourceReset eInit)
{
    if(eInit)
    {
        constexpr coreUint32 aiQuadData[] = {coreVector2(-0.5f, 0.5f).PackSnorm2x16(),
                                             coreVector2(-0.5f,-0.5f).PackSnorm2x16(),
                                             coreVector2( 0.5f, 0.5f).PackSnorm2x16(),
                                             coreVector2( 0.5f,-0.5f).PackSnorm2x16()};

        constexpr coreVector2 avTriangleData[] = {coreVector2(-0.5f, 0.5f),
                                                  coreVector2(-0.5f,-1.5f),
                                                  coreVector2( 1.5f, 0.5f)};

        // create low-memory square model
        m_pLowQuad->SetBoundingRange (coreVector3(0.5f,0.5f,0.0f));
        m_pLowQuad->SetBoundingRadius(1.0f / SQRT2);
        m_pLowQuad->SetPrimitiveType (GL_TRIANGLE_STRIP);

        // define vertex data
        coreVertexBuffer* pBuffer = m_pLowQuad->CreateVertexBuffer(ARRAY_SIZE(aiQuadData), sizeof(coreUint32), aiQuadData, CORE_DATABUFFER_STORAGE_STATIC);
        pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 2u, GL_SHORT, false, 0u);

        // create low-memory triangle model
        m_pLowTriangle->SetBoundingRange (coreVector3(1.5f,1.5f,0.0f));
        m_pLowTriangle->SetBoundingRadius(SQRT(2.5f));
        m_pLowTriangle->SetPrimitiveType (GL_TRIANGLES);

        // define vertex data
        pBuffer = m_pLowTriangle->CreateVertexBuffer(ARRAY_SIZE(avTriangleData), sizeof(coreVector2), avTriangleData, CORE_DATABUFFER_STORAGE_STATIC);
        pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 2u, GL_FLOAT, false, 0u);

        Core::Log->Info("Low-Memory models created");

        // create frame buffer fallback
        if(!CORE_GL_SUPPORT(EXT_framebuffer_blit))
        {
            m_pBlitFallback = new coreObject2D();
            m_pBlitFallback->DefineProgram("default_2d_program");

            Core::Log->Warning("Frame buffer fallback created");
        }

        // refresh all existing 2d-objects
        this->RefreshSprites();
    }
    else
    {
        // unload low-memory models
        m_pLowQuad    ->Unload();
        m_pLowTriangle->Unload();

        // delete frame buffer fallback
        SAFE_DELETE(m_pBlitFallback)
    }
}


// ****************************************************************
/* update all objects and collisions */
void coreObjectManager::__UpdateObjects()
{
    if(m_bIndexDirty)
    {
        m_bIndexDirty = false;

        // clear index
        m_aiIndex.clear();

        // loop through all objects
        FOR_EACH(it, m_aapObjectList)
        {
            FOR_EACH_DYN(et, *it)
            {
                // check for invalid pointers and remove them
                if(*et) DYN_KEEP  (et)
                   else DYN_REMOVE(et, *it)
            }

            // reconstruct index
            for(coreUintW i = 0u, ie = it->size(); i < ie; ++i)
                m_aiIndex.emplace_bs((*it)[i], i);
        }
    }

    // loop through all collisions
    const coreUint32 iCurFrame = Core::System->GetCurFrame();
    FOR_EACH_DYN(it, m_aiObjectCollision)
    {
        // check for old entries and remove them
        if((*it) == iCurFrame) DYN_KEEP  (it)
                          else DYN_REMOVE(it, m_aiObjectCollision)
    }
}


// ****************************************************************
/* bind 3d-object to type */
void coreObjectManager::__BindObject(coreObject3D* pObject, const coreInt32 iType)
{
    ASSERT(pObject && iType && !m_aiIndex.count_bs(pObject))

#if defined(_CORE_DEBUG_)

    // check for duplicate objects
    FOR_EACH(it, m_aapObjectList)
        FOR_EACH(et, *it)
            ASSERT(*et != pObject)

#endif

    // get requested list
    coreObjectList& oList = m_aapObjectList[iType];

    // add object and index
    m_aiIndex.emplace_bs(pObject, oList.size());
    oList.push_back(pObject);
}


// ****************************************************************
/* unbind 3d-object from type */
void coreObjectManager::__UnbindObject(coreObject3D* pObject, const coreInt32 iType)
{
    ASSERT(pObject && iType && m_aiIndex.count_bs(pObject))

    const auto it = m_aiIndex.find_bs(pObject);

    // remove object and index
    m_aapObjectList.at(iType)[*it] = NULL;
    m_aiIndex.erase(it);

    // request index reconstruction
    m_bIndexDirty = true;
}


// ****************************************************************
/* handle and track new collisions */
coreBool coreObjectManager::__NewCollision(const coreObject3D* pObject1, const coreObject3D* pObject2)
{
    // create lookup key
    coreObjectCollision oCollision;
    oCollision.pObject1 = pObject1;
    oCollision.pObject2 = pObject2;

    // find existing collision
    if(m_aiObjectCollision.count_bs(oCollision))
    {
        // update frame number
        m_aiObjectCollision.at_bs(oCollision) = Core::System->GetCurFrame();
        return false;
    }

    // add collision to list
    m_aiObjectCollision.emplace_bs(oCollision, Core::System->GetCurFrame());
    return true;
}