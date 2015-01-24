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
    struct coreNode
    {
        T     tPosition;   //!< position of the node
        T     tTangent;    //!< tangent of the node
        float fDistance;   //!< distance from this node to the next (0.0f = last or spiky node)

        constexpr_func coreNode()noexcept;
    };


private:
    std::vector<coreNode> m_apNode;   //!< nodes of the spline
    float m_fTotalDistance;           //!< approximated total distance


public:
    coreSpline()noexcept;
    coreSpline(const coreSpline<T>& c)noexcept;
    coreSpline(coreSpline<T>&&      m)noexcept;
    ~coreSpline();

    /*! assignment operations */
    //! @{
    coreSpline<T>& operator = (coreSpline<T> o)noexcept;
    template <typename S> friend void swap(coreSpline<S>& a, coreSpline<S>& b)noexcept;
    //! @}

    /*! manage nodes */
    //! @{
    void AddNode   (const T& tPosition, const T& tTangent);
    void AddNodes  (const T& tPosition, const T& tTangentIn, const T& tTangentOut);
    void AddLoop   ();
    void DeleteNode(const coreUint& iIndex);
    void ClearNodes();
    //! @}

    /*! edit node properties */
    //! @{
    void EditNodePosition(const coreUint& iIndex, const T& tNewPosition);
    void EditNodeTangent (const coreUint& iIndex, const T& tNewTangent);
    //! @}

    /*! calculate position and direction */
    //! @{
    void     CalcPosDir   (const float& fDistance, T* OUTPUT ptPosition, T* OUTPUT ptDirection)const;
    inline T CalcPosition (const float& fDistance)const {T tPos; this->CalcPosDir(fDistance, &tPos, NULL); return tPos;}
    inline T CalcDirection(const float& fDistance)const {T tDir; this->CalcPosDir(fDistance, NULL, &tDir); return tDir;}
    //! @}

    /*! translate distance into relative node index and time */
    //! @{
    void TranslateRelative(const float& fDistance, coreUint* OUTPUT piRelIndex, float* OUTPUT pfRelTime)const;
    //! @}

    /*! get object properties */
    //! @{
    inline const coreNode& GetNode         (const coreUint& iIndex)const {return m_apNode.at(iIndex);}
    inline coreUint        GetNumNodes     ()const                       {return coreUint(m_apNode.size());}
    inline const float&    GetTotalDistance()const                       {return m_fTotalDistance;}
    //! @}


private:
    /*! calculate final position and direction */
    //! @{
    static void __CalcPosDir(const float& fTime, const T& tP1, const T& tP2, const T& tT1, const T& tT2, T* OUTPUT ptPosition, T* OUTPUT ptDirection);
    //! @}
};


// ****************************************************************
/* constructor */
template <typename T> constexpr_func coreSpline<T>::coreNode::coreNode()noexcept
: tPosition (T())
, tTangent  (T())
, fDistance (0.0f)
{
}


// ****************************************************************
/* constructor */
template <typename T> coreSpline<T>::coreSpline()noexcept
: m_fTotalDistance (0.0f)
{
}

template <typename T> coreSpline<T>::coreSpline(const coreSpline<T>& c)noexcept
: m_apNode         (c.m_apNode)
, m_fTotalDistance (c.m_fTotalDistance)
{
}

template <typename T> coreSpline<T>::coreSpline(coreSpline<T>&& m)noexcept
: m_apNode         (std::move(m.m_apNode))
, m_fTotalDistance (m.m_fTotalDistance)
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
/* assignment operations */
template <typename T> coreSpline<T>& coreSpline<T>::operator = (coreSpline<T> o)noexcept
{
    swap(*this, o);
    return *this;
}

template <typename S> void swap(coreSpline<S>& a, coreSpline<S>& b)noexcept
{
    using std::swap;
    swap(a.m_apNode,         b.m_apNode);
    swap(a.m_fTotalDistance, b.m_fTotalDistance);
}


// ****************************************************************
/* add node to spline */
template <typename T> void coreSpline<T>::AddNode(const T& tPosition, const T& tTangent)
{
    ASSERT(tTangent.IsNormalized())

    // create new node
    coreNode oNewNode;
    oNewNode.tPosition = tPosition;
    oNewNode.tTangent  = tTangent;

    if(!m_apNode.empty())
    {
        coreNode& oLastNode = m_apNode.back();

        // edit last node and total distance
        oLastNode.fDistance = (oNewNode.tPosition - oLastNode.tPosition).Length();
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
/* convert spline into a closed circuit */
template <typename T> void coreSpline<T>::AddLoop()
{
    WARN_IF(m_apNode.empty()) return;

    // append copy of first node to the end
    const coreNode& oFirstNode = m_apNode.front();
    this->AddNode(oFirstNode.tPosition, oFirstNode.tTangent);
}


// ****************************************************************
/* remove node from spline */
template <typename T> void coreSpline<T>::DeleteNode(const coreUint& iIndex)
{
    WARN_IF(iIndex >= coreUint(m_apNode.size())) return;

    if(iIndex)
    {
        const coreUint iNextIndex = iIndex + 1;
        coreNode&      oPrevNode  = m_apNode[iIndex - 1];

        // calculate new distance between previous and next node
        const float fNewDistance = (iNextIndex >= coreUint(m_apNode.size())) ? 0.0f : (m_apNode[iNextIndex].tPosition - oPrevNode.tPosition).Length();

        // add new distance and remove both old distances
        m_fTotalDistance   += fNewDistance - oPrevNode.fDistance - m_apNode[iIndex].fDistance;
        oPrevNode.fDistance = fNewDistance;
    }
    else m_fTotalDistance -= m_apNode[iIndex].fDistance;

    // remove old node
    m_apNode.erase(m_apNode.begin()+iIndex);
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
template <typename T> void coreSpline<T>::EditNodePosition(const coreUint& iIndex, const T& tNewPosition)
{
    ASSERT(iIndex < coreUint(m_apNode.size()))

    // set new node position
    coreNode& oCurNode = m_apNode[iIndex];
    oCurNode.tPosition = tNewPosition;

    // update relation to next node
    const coreUint iNextIndex = iIndex + 1;
    if(iNextIndex < coreUint(m_apNode.size()))
    {
        const coreNode& oNextNode = m_apNode[iNextIndex];

        // calculate new distance between current and next node
        const float fNewDistance = (oNextNode.tPosition - oCurNode.tPosition).Length();

        // edit current node and total distance
        m_fTotalDistance  += fNewDistance - oCurNode.fDistance;
        oCurNode.fDistance = fNewDistance;
    }

    // update relation to previous node
    if(iIndex)
    {
        coreNode& oPrevNode = m_apNode[iIndex - 1];

        // calculate new distance between previous and current node
        const float fNewDistance = (oCurNode.tPosition - oPrevNode.tPosition).Length();

        // edit previous node and total distance
        m_fTotalDistance   += fNewDistance - oPrevNode.fDistance;
        oPrevNode.fDistance = fNewDistance;
    }
}


// ****************************************************************
/* edit node tangent */
template <typename T> void coreSpline<T>::EditNodeTangent(const coreUint& iIndex, const T& tNewTangent)
{
    ASSERT((iIndex < coreUint(m_apNode.size())) && tNewTangent.IsNormalized())

    // only set new node tangent
    m_apNode[iIndex].tTangent = tNewTangent;
}


// ****************************************************************
/* calculate position and direction */
template <typename T> void coreSpline<T>::CalcPosDir(const float& fDistance, T* OUTPUT ptPosition, T* OUTPUT ptDirection)const
{
    ASSERT(ptPosition || ptDirection)

    // translate distance into relative node index and time
    coreUint iRelIndex;
    float    fRelTime;
    this->TranslateRelative(fDistance, &iRelIndex, &fRelTime);

    // get both enclosing nodes
    const coreNode& oCurNode  = m_apNode[iRelIndex];
    const coreNode& oNextNode = m_apNode[iRelIndex + 1];

    // scale tangents of both nodes with distance between them
    const T tCurVelocity  = oCurNode .tTangent * oCurNode.fDistance;
    const T tNextVelocity = oNextNode.tTangent * oCurNode.fDistance;

    // calculate final position and direction
    coreSpline::__CalcPosDir(fRelTime, oCurNode.tPosition, oNextNode.tPosition, tCurVelocity, tNextVelocity, ptPosition, ptDirection);
}


// ****************************************************************
/* translate distance into relative node index and time */
template <typename T> void coreSpline<T>::TranslateRelative(const float& fDistance, coreUint* OUTPUT piRelIndex, float* OUTPUT pfRelTime)const
{
    ASSERT(piRelIndex && pfRelTime && (m_apNode.size() >= 2))

    coreUint       iCurIndex    = 0;
    float          fCurDistance = 0.0f;
    const coreUint iMaxIndex    = coreUint(m_apNode.size()) - 2;
    const float    fMaxDistance = MIN(fDistance, m_fTotalDistance);

    // search for first relative node
    float fNewDistance;
    while((iCurIndex < iMaxIndex) && ((fNewDistance = fCurDistance + m_apNode[iCurIndex].fDistance) < fMaxDistance))
    {
        ++iCurIndex;
        fCurDistance = fNewDistance;
    }

    // save index and calculate relative time to the next node (normalized linear difference)
    *piRelIndex =  iCurIndex;
    *pfRelTime  = (fMaxDistance - fCurDistance) * RCP(m_apNode[iCurIndex].fDistance);
}


// ****************************************************************
/* calculate final position and direction */
template <typename T> void coreSpline<T>::__CalcPosDir(const float& fTime, const T& tP1, const T& tP2, const T& tT1, const T& tT2, T* OUTPUT ptPosition, T* OUTPUT ptDirection)
{
    // normal calculation:
    // *ptPosition  = (2.0f*X3 - 3.0f*X2 + 1.0f)*tP1 - (2.0f*X3 - 3.0f*X2)*tP2 + (     X3 - 2.0f*X2 +   X1)*tT1 + (     X3 -      X2)*tT2;
    // *ptDirection = (6.0f*X2 - 6.0f*X1       )*tP1 - (6.0f*X2 - 6.0f*X1)*tP2 + (3.0f*X2 - 4.0f*X1 + 1.0f)*tT1 + (3.0f*X2 - 2.0f*X1)*tT2;

    const float& X1 = fTime;
    const float  X2 =   X1 *  X1;
    const float  D  = 3.0f *  X2;
    const float  E  =   X2 -  X1;
    const T      PP =  tP1 - tP2;

    if(ptPosition)
    {
        // use default cubic Hermite interpolation
        const float X3 = X2 * X1;
        const float A  = X3 - X2;
        *ptPosition    = (2.0f * X3 - D)*PP + tP1 + (A - E)*tT1 + (A)*tT2;
    }

    if(ptDirection)
    {
        // use first derivation to get tangent value (and normalize it)
        const float B = 2.0f * X1;
        const float C =    D -  B;
        *ptDirection  = ((6.0f * E)*PP + (C - B + 1.0f)*tT1 + (C)*tT2).Normalize();
    }
}


// ****************************************************************
/* default spline types */
typedef coreSpline<coreVector2> coreSpline2;
typedef coreSpline<coreVector3> coreSpline3;


#endif /* _CORE_GUARD_SPLINE_H_ */