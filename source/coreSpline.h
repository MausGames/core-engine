#pragma once
// TODO extend to NURBS


// ****************************************************************
// spline class
class coreSpline
{
private:
    // node struct
    struct coreNode
    {
        coreVector3 vPosition;   // position of the node
        coreVector3 vTangent;    // tangent of the node
        float fDistance;         // distance from this node to the next (0 = last node)
    };


private:
    std::vector<coreNode> m_apNode;   // nodes of the spline
    float m_fMaxDistance;             // approximated maximum distance


public:
    coreSpline();
    ~coreSpline();

    // control nodes
    void AddNode(const coreVector3& vPosition, const coreVector3& vTangent);
    void RemoveNode(const coreUint& iIndex);
    void ClearNodes();

    // get position and direction
    coreVector3 GetPosition(const float& fTime)const;
    coreVector3 GetPosition(const float& fTime, const coreVector3& vP1, const coreVector3& vP2, const coreVector3& vT1, const coreVector3& vT2)const;
    coreVector3 GetDirection(const float& fTime)const;
    coreVector3 GetDirection(const float& fTime, const coreVector3& vP1, const coreVector3& vP2, const coreVector3& vT1, const coreVector3& vT2)const;

    // get relative node and time 
    void GetRelative(const float& fTime, coreUint* piIndex, float* pfRelative)const;

    // get distance
    inline const float& GetDistance()const            {return m_fMaxDistance;}
    inline float GetDistance(const float& fTime)const {return fTime*m_fMaxDistance;}
};