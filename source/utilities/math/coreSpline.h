//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SPLINE_H_
#define _CORE_GUARD_SPLINE_H_

// TODO: implement insert-node function
// TODO: multidimensional spline ((spline-)interpolation between other splines)


// ****************************************************************
/* non-uniform spline class */
template <typename T> class coreSpline final
{
private:
    /*! node structure */
    struct coreNode final
    {
        T         tPosition;   //!< position of the node
        T         tTangent;    //!< tangent of the node
        coreFloat fDistance;   //!< distance from this node to the next (0.0f = last or spiky node)
    };


private:
    std::vector<coreNode> m_apNode;   //!< nodes of the spline
    coreFloat m_fTotalDistance;       //!< approximated total distance


public:
    coreSpline()noexcept;
    ~coreSpline();

    ENABLE_COPY(coreSpline)

    /*! manage nodes */
    //! @{
    void AddNode   (const T& tPosition, const T& tTangent);
    void AddNodes  (const T& tPosition, const T& tTangentIn, const T& tTangentOut);
    void AddStop   (const T& tPosition, const T& tTangent = T({}) + 1.0f);
    void AddLoop   ();
    void DeleteNode(const coreUintW iIndex);
    void ClearNodes();
    //! @}

    /*! edit node properties */
    //! @{
    void EditNodePosition(const coreUintW iIndex, const T& tNewPosition);
    void EditNodeTangent (const coreUintW iIndex, const T& tNewTangent);
    //! @}

    /*! refine existing nodes for improved interpolation */
    //! @{
    void Refine();
    //! @}

    /*! calculate position and direction */
    //! @{
    void     CalcPosDir   (const coreFloat fDistance, T* OUTPUT ptPosition, T* OUTPUT ptDirection)const;
    inline T CalcPosition (const coreFloat fDistance)const {T tPos; this->CalcPosDir(fDistance, &tPos, NULL); return tPos;}
    inline T CalcDirection(const coreFloat fDistance)const {T tDir; this->CalcPosDir(fDistance, NULL, &tDir); return tDir;}
    //! @}

    /*! translate between distance and relative node index and time */
    //! @{
    void      TranslateRelative(const coreFloat fDistance, coreUintW* OUTPUT piRelIndex, coreFloat* OUTPUT pfRelTime)const;
    coreFloat TranslateDistance(const coreUintW iRelIndex, const coreFloat fRelTime)const;
    //! @}

    /*! get object properties */
    //! @{
    inline const coreNode&  GetNode         (const coreUintW iIndex)const {ASSERT(iIndex < m_apNode.size()) return m_apNode[iIndex];}
    inline       coreUintW  GetNumNodes     ()const                       {return m_apNode.size();}
    inline const coreFloat& GetTotalDistance()const                       {return m_fTotalDistance;}
    //! @}


private:
    /*! calculate final position and direction */
    //! @{
    static FUNC_NOALIAS void __CalcPosDir(const coreFloat fTime, const T& tP1, const T& tP2, const T& tT1, const T& tT2, T* OUTPUT ptPosition, T* OUTPUT ptDirection);
    //! @}
};


// ****************************************************************
/* constructor */
template <typename T> coreSpline<T>::coreSpline()noexcept
: m_apNode         {}
, m_fTotalDistance (0.0f)
{
}


// ****************************************************************
/* destructor */
template <typename T> coreSpline<T>::~coreSpline()
{
    // remove all nodes
    this->ClearNodes();
}


// ****************************************************************
/* add node to spline */
template <typename T> void coreSpline<T>::AddNode(const T& tPosition, const T& tTangent)
{
    // create new node
    coreNode oNewNode;
    oNewNode.tPosition = tPosition;
    oNewNode.tTangent  = tTangent;
    oNewNode.fDistance = 0.0f;

    if(!m_apNode.empty())
    {
        coreNode& oLastNode = m_apNode.back();

        // edit last node and total distance
        oLastNode.fDistance = (oNewNode.tPosition - oLastNode.tPosition).Length() * RCP((oNewNode.tTangent.Length() + oLastNode.tTangent.Length()) * 0.5f);
        m_fTotalDistance   += oLastNode.fDistance;
    }

    // add new node
    m_apNode.push_back(oNewNode);
}


// ****************************************************************
/* add hard corner node to spline */
template <typename T> void coreSpline<T>::AddNodes(const T& tPosition, const T& tTangentIn, const T& tTangentOut)
{
    // actually add two nodes at the same position
    this->AddNode(tPosition, tTangentIn);
    this->AddNode(tPosition, tTangentOut);
}


// ****************************************************************
/* add stopping node to spline */
template <typename T> void coreSpline<T>::AddStop(const T& tPosition, const T& tTangent)
{
    // actually add node with zero-length tangent
    this->AddNode(tPosition, tTangent * CORE_MATH_PRECISION);
}


// ****************************************************************
/* convert spline into a closed circuit */
template <typename T> void coreSpline<T>::AddLoop()
{
    ASSERT(!m_apNode.empty())

    // append copy of first node to the end
    const coreNode& oFirstNode = m_apNode.front();
    this->AddNode(oFirstNode.tPosition, oFirstNode.tTangent);
}


// ****************************************************************
/* remove node from spline */
template <typename T> void coreSpline<T>::DeleteNode(const coreUintW iIndex)
{
    ASSERT(iIndex < m_apNode.size())

    if(iIndex)
    {
        const coreUintW iNextIndex = iIndex + 1u;
        coreNode&       oPrevNode  = m_apNode[iIndex - 1u];

        // calculate new distance between previous and next node
        const coreFloat fNewDistance = (iNextIndex >= m_apNode.size()) ? 0.0f : (m_apNode[iNextIndex].tPosition - oPrevNode.tPosition).Length();

        // add new distance and remove both old distances
        m_fTotalDistance   += fNewDistance - oPrevNode.fDistance - m_apNode[iIndex].fDistance;
        oPrevNode.fDistance = fNewDistance;
    }
    else m_fTotalDistance -= m_apNode[iIndex].fDistance;

    // remove old node
    m_apNode.erase(m_apNode.begin() + iIndex);
}


// ****************************************************************
/* remove all nodes */
template <typename T> void coreSpline<T>::ClearNodes()
{
    // clear memory
    m_apNode.clear();

    // reset total distance
    m_fTotalDistance = 0.0f;
}


// ****************************************************************
/* edit node position */
template <typename T> void coreSpline<T>::EditNodePosition(const coreUintW iIndex, const T& tNewPosition)
{
    ASSERT(iIndex < m_apNode.size())

    // set new node position
    coreNode& oCurNode = m_apNode[iIndex];
    oCurNode.tPosition = tNewPosition;

    // update relation to next node
    const coreUintW iNextIndex = iIndex + 1u;
    if(iNextIndex < m_apNode.size())
    {
        const coreNode& oNextNode = m_apNode[iNextIndex];

        // calculate new distance between current and next node
        const coreFloat fNewDistance = (oNextNode.tPosition - oCurNode.tPosition).Length();

        // edit current node and total distance
        m_fTotalDistance  += fNewDistance - oCurNode.fDistance;
        oCurNode.fDistance = fNewDistance;
    }

    // update relation to previous node
    if(iIndex)
    {
        coreNode& oPrevNode = m_apNode[iIndex - 1u];

        // calculate new distance between previous and current node
        const coreFloat fNewDistance = (oCurNode.tPosition - oPrevNode.tPosition).Length();

        // edit previous node and total distance
        m_fTotalDistance   += fNewDistance - oPrevNode.fDistance;
        oPrevNode.fDistance = fNewDistance;
    }
}


// ****************************************************************
/* edit node tangent */
template <typename T> void coreSpline<T>::EditNodeTangent(const coreUintW iIndex, const T& tNewTangent)
{
    ASSERT(iIndex < m_apNode.size())

    // only set new node tangent
    m_apNode[iIndex].tTangent = tNewTangent;
}


// ****************************************************************
/* refine existing nodes for improved interpolation */
template <typename T> void coreSpline<T>::Refine()
{
    ASSERT(!m_apNode.empty())

    // reset total distance
    m_fTotalDistance = 0.0f;

    for(coreUintW i = 0u, ie = m_apNode.size() - 1u; i < ie; ++i)
    {
        // get both enclosing nodes
        coreNode& oCurNode  = m_apNode[i];
        coreNode& oNextNode = m_apNode[i + 1u];

        // scale tangents of both nodes with distance between them
        const T tCurVelocity  = oCurNode .tTangent * oCurNode.fDistance;
        const T tNextVelocity = oNextNode.tTangent * oCurNode.fDistance;

        // calculate points on current approximation (to get better distances and tangent velocities)
        T B, C, D;
        const T& A = oCurNode .tPosition;
        const T& E = oNextNode.tPosition;
        coreSpline::__CalcPosDir(0.25f, A, E, tCurVelocity, tNextVelocity, &B, NULL);
        coreSpline::__CalcPosDir(0.50f, A, E, tCurVelocity, tNextVelocity, &C, NULL);
        coreSpline::__CalcPosDir(0.75f, A, E, tCurVelocity, tNextVelocity, &D, NULL);

        // edit current node and total distance
        oCurNode.fDistance = ((E - D).Length() +
                              (D - C).Length() +
                              (C - B).Length() +
                              (B - A).Length()) * RCP((oNextNode.tTangent.Length() + oCurNode.tTangent.Length()) * 0.5f);
        m_fTotalDistance  += oCurNode.fDistance;
    }

    // reduce memory consumption
    m_apNode.shrink_to_fit();
}


// ****************************************************************
/* calculate position and direction */
template <typename T> void coreSpline<T>::CalcPosDir(const coreFloat fDistance, T* OUTPUT ptPosition, T* OUTPUT ptDirection)const
{
    ASSERT(ptPosition || ptDirection)

    // translate distance into relative node index and time
    coreUintW iRelIndex;
    coreFloat fRelTime;
    this->TranslateRelative(fDistance, &iRelIndex, &fRelTime);

    // get both enclosing nodes
    const coreNode& oCurNode  = m_apNode[iRelIndex];
    const coreNode& oNextNode = m_apNode[iRelIndex + 1u];

    // scale tangents of both nodes with distance between them
    const T tCurVelocity  = oCurNode .tTangent * oCurNode.fDistance;
    const T tNextVelocity = oNextNode.tTangent * oCurNode.fDistance;

    // calculate final position and direction
    coreSpline::__CalcPosDir(fRelTime, oCurNode.tPosition, oNextNode.tPosition, tCurVelocity, tNextVelocity, ptPosition, ptDirection);
}


// ****************************************************************
/* translate distance into relative node index and time */
template <typename T> void coreSpline<T>::TranslateRelative(const coreFloat fDistance, coreUintW* OUTPUT piRelIndex, coreFloat* OUTPUT pfRelTime)const
{
    ASSERT((fDistance >= 0.0f) && (fDistance <= m_fTotalDistance) && piRelIndex && pfRelTime && (m_apNode.size() >= 2u))

    coreUintW       iCurIndex    = 0u;
    coreFloat       fCurDistance = 0.0f;
    const coreUintW iMaxIndex    = m_apNode.size() - 2u;
    const coreFloat fMaxDistance = fDistance;

    // search for first relative node
    coreFloat fNewDistance;
    while((iCurIndex < iMaxIndex) && ((fNewDistance = fCurDistance + m_apNode[iCurIndex].fDistance) < fMaxDistance))
    {
        ++iCurIndex;
        fCurDistance = fNewDistance;
    }

    // save index and calculate relative time to the next node (normalized linear difference)
    (*piRelIndex) =  iCurIndex;
    (*pfRelTime)  = (fMaxDistance - fCurDistance) * RCP(m_apNode[iCurIndex].fDistance);
}


// ****************************************************************
/* translate relative node index and time into distance */
template <typename T> coreFloat coreSpline<T>::TranslateDistance(const coreUintW iRelIndex, const coreFloat fRelTime)const
{
    ASSERT((iRelIndex < m_apNode.size()) && (fRelTime >= 0.0f) && (fRelTime <= 1.0f))

    // find total distance to relative node
    coreFloat fOutput = 0.0f;
    for(coreUintW i = 0u; i < iRelIndex; ++i) fOutput += m_apNode[i].fDistance;

    // add scaled distance to next node
    return fOutput + (m_apNode[iRelIndex].fDistance * fRelTime);
}


// ****************************************************************
/* calculate final position and direction */
template <typename T> FUNC_NOALIAS void coreSpline<T>::__CalcPosDir(const coreFloat fTime, const T& tP1, const T& tP2, const T& tT1, const T& tT2, T* OUTPUT ptPosition, T* OUTPUT ptDirection)
{
    // normal calculation:
    // (*ptPosition)  = (2.0f*X3 - 3.0f*X2 + 1.0f)*tP1 - (2.0f*X3 - 3.0f*X2)*tP2 + (     X3 - 2.0f*X2 +   X1)*tT1 + (     X3 -      X2)*tT2;
    // (*ptDirection) = (6.0f*X2 - 6.0f*X1       )*tP1 - (6.0f*X2 - 6.0f*X1)*tP2 + (3.0f*X2 - 4.0f*X1 + 1.0f)*tT1 + (3.0f*X2 - 2.0f*X1)*tT2;

    const coreFloat X1 = fTime;
    const coreFloat X2 =   X1 *  X1;
    const coreFloat D  = 3.0f *  X2;
    const coreFloat E  =   X2 -  X1;
    const T         PP =  tP1 - tP2;

    if(ptPosition)
    {
        // use default cubic Hermite interpolation
        const coreFloat X3 = X2 * X1;
        const coreFloat A  = X3 - X2;
        (*ptPosition) = (2.0f * X3 - D)*PP + tP1 + (A - E)*tT1 + (A)*tT2;
    }

    if(ptDirection)
    {
        // use first derivation to get tangent value (and normalize it)
        const coreFloat B = 2.0f * X1;
        const coreFloat C =    D -  B;
        (*ptDirection) = ((6.0f * E)*PP + (C - B + 1.0f)*tT1 + (C)*tT2).Normalized();
    }
}


// ****************************************************************
/* default spline types */
using coreSpline2 = coreSpline<coreVector2>;
using coreSpline3 = coreSpline<coreVector3>;
using coreSpline4 = coreSpline<coreVector4>;


#endif /* _CORE_GUARD_SPLINE_H_ */