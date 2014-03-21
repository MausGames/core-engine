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


// ****************************************************************
// spline class
// TODO: extend to NURBS
class coreSpline final
{
private:
    //! node structure
    struct coreNode
    {
        coreVector3 vPosition;   //!< position of the node
        coreVector3 vTangent;    //!< tangent of the node
        float fDistance;         //!< distance from this node to the next (0 = last node)

        constexpr_func coreNode()noexcept;
    };


private:
    std::vector<coreNode> m_apNode;   //!< nodes of the spline
    float m_fMaxDistance;             //!< approximated maximum distance


public:
    coreSpline()noexcept;
    coreSpline(const coreSpline& c)noexcept;
    coreSpline(coreSpline&& m)noexcept;
    ~coreSpline();

    //! assignment operator
    //! @{
    coreSpline& operator = (coreSpline o)noexcept;
    friend void swap(coreSpline& a, coreSpline& b)noexcept;
    //! @}

    //! manage nodes
    //! @{
    void AddNode(const coreVector3& vPosition, const coreVector3& vTangent);
    void DeleteNode(const coreUint& iIndex);
    void ClearNodes();
    //! @}

    //! get position and direction
    //! @{
    coreVector3 GetPosition(const float& fTime)const;
    coreVector3 GetDirection(const float& fTime)const;
    static constexpr_func coreVector3 GetPosition(const float& fTime, const coreVector3& vP1, const coreVector3& vP2, const coreVector3& vT1, const coreVector3& vT2)noexcept;
    static constexpr_func coreVector3 GetDirection(const float& fTime, const coreVector3& vP1, const coreVector3& vP2, const coreVector3& vT1, const coreVector3& vT2)noexcept;
    //! @}

    //! get relative node and time
    //! @{
    void GetRelative(const float& fTime, coreUint* piIndex, float* pfRelative)const;
    //! @}

    //! get distance
    //! @{
    inline const float& GetDistance()const            {return m_fMaxDistance;}
    inline float GetDistance(const float& fTime)const {return fTime*m_fMaxDistance;}
    //! @}
};


// ****************************************************************
// constructor
constexpr_func coreSpline::coreNode::coreNode()noexcept
: vPosition (coreVector3(0.0f,0.0f,0.0f))
, vTangent  (coreVector3(0.0f,0.0f,0.0f))
, fDistance (0.0f)
{
}


// ****************************************************************
// get position in spline
constexpr_func coreVector3 coreSpline::GetPosition(const float& fTime, const coreVector3& vP1, const coreVector3& vP2, const coreVector3& vT1, const coreVector3& vT2)noexcept
{
    // calculate position with cubic function
    return ((( 2.0f*vP1 - 2.0f*vP2 +      vT1 + vT2)  * fTime +
             (-3.0f*vP1 + 3.0f*vP2 - 2.0f*vT1 - vT2)) * fTime + vT1) * fTime + vP1;
}


// ****************************************************************
// get direction in spline
constexpr_func coreVector3 coreSpline::GetDirection(const float& fTime, const coreVector3& vP1, const coreVector3& vP2, const coreVector3& vT1, const coreVector3& vT2)noexcept
{
    // calculate direction with quadratic function
    return (( 2.0f*vP1 - 2.0f*vP2 +      vT1 + vT2)  * fTime * 1.5f +
            (-3.0f*vP1 + 3.0f*vP2 - 2.0f*vT1 - vT2)) * fTime * 2.0f + vT1;
}


#endif // _CORE_GUARD_SPLINE_H_