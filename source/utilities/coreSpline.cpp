//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreSpline::coreSpline()
: m_fMaxDistance (0.0f)
{
    // reserve memory for nodes
    m_apNode.reserve(32);
}

coreSpline::coreSpline(const coreSpline& c)
: m_apNode       (c.m_apNode)
, m_fMaxDistance (c.m_fMaxDistance)
{
}

coreSpline::coreSpline(coreSpline&& m)
: m_apNode       (std::move(m.m_apNode))
, m_fMaxDistance (m.m_fMaxDistance)
{
}


// ****************************************************************
// destructor
coreSpline::~coreSpline()
{
    this->ClearNodes();
}


// ****************************************************************
// assignment operators
coreSpline& coreSpline::operator = (coreSpline o)
{
    swap(*this, o);
    return *this;
}

void swap(coreSpline& a, coreSpline& b)
{
    using std::swap;
    swap(a.m_apNode,       b.m_apNode);
    swap(a.m_fMaxDistance, b.m_fMaxDistance);
}


// ****************************************************************
// add node to spline
void coreSpline::AddNode(const coreVector3& vPosition, const coreVector3& vTangent)
{
    // create new node
    coreNode NewNode;
    NewNode.vPosition = vPosition;
    NewNode.vTangent  = vTangent.Normalized();
    NewNode.fDistance = 0.0f;

    // edit last node and increase max distance
    if(!m_apNode.empty())
    {
        coreNode* pLastNode = &m_apNode.back();

        pLastNode->fDistance = (NewNode.vPosition - pLastNode->vPosition).Length();
        m_fMaxDistance      += pLastNode->fDistance;
    }

    // add new node
    m_apNode.push_back(NewNode);
}


// ****************************************************************
// remove node from spline
void coreSpline::RemoveNode(const coreUint& iIndex)
{
    // TODO: implement function
}


// ****************************************************************
// remove all nodes
void coreSpline::ClearNodes()
{
    // clear memory
    m_apNode.clear();

    // reset distance
    m_fMaxDistance = 0.0f;
}


// ****************************************************************
// get position in spline
coreVector3 coreSpline::GetPosition(const float& fTime)const
{
    coreUint iIndex;
    float fRelative;

    // get relative node and time
    this->GetRelative(fTime, &iIndex, &fRelative);

    // scale velocity of the nodes
    const coreVector3 vStartVel = m_apNode[iIndex].vTangent * m_apNode[iIndex].fDistance;
    const coreVector3 vEndVel = m_apNode[iIndex+1].vTangent * m_apNode[iIndex].fDistance;

    return this->GetPosition(fRelative, m_apNode[iIndex].vPosition, m_apNode[iIndex+1].vPosition, vStartVel, vEndVel);
}

coreVector3 coreSpline::GetPosition(const float& fTime, const coreVector3& vP1, const coreVector3& vP2, const coreVector3& vT1, const coreVector3& vT2)const
{
    // calculate position with cubic function
    return ( 2.0f*vP1 - 2.0f*vP2 +      vT1 + vT2) * fTime*fTime*fTime +
           (-3.0f*vP1 + 3.0f*vP2 - 2.0f*vT1 - vT2) * fTime*fTime + vT1*fTime + vP1;
}


// ****************************************************************
// get direction in spline
coreVector3 coreSpline::GetDirection(const float& fTime)const
{
    coreUint iIndex;
    float fRelative;

    // get relative node and time
    this->GetRelative(fTime, &iIndex, &fRelative);

    // scale velocity of the nodes
    const coreVector3 vStartVel = m_apNode[iIndex].vTangent * m_apNode[iIndex].fDistance;
    const coreVector3 vEndVel = m_apNode[iIndex+1].vTangent * m_apNode[iIndex].fDistance;

    return this->GetDirection(fRelative, m_apNode[iIndex].vPosition, m_apNode[iIndex+1].vPosition, vStartVel, vEndVel).Normalize();
}

coreVector3 coreSpline::GetDirection(const float& fTime, const coreVector3& vP1, const coreVector3& vP2, const coreVector3& vT1, const coreVector3& vT2)const
{
    // calculate direction with cubic function
    return ( 2.0f*vP1 - 2.0f*vP2 +      vT1 + vT2) * fTime*fTime * 3.0f +
           (-3.0f*vP1 + 3.0f*vP2 - 2.0f*vT1 - vT2) * fTime * 2.0f + vT1;
}


// ****************************************************************
// get relative node and time
void coreSpline::GetRelative(const float& fTime, coreUint* piIndex, float* pfRelative)const
{
    float fDistance = this->GetDistance(fTime);
    float fCurDistance = 0.0f;

    // search relevant node
    *piIndex = 0;
    while(fCurDistance + m_apNode[*piIndex].fDistance < fDistance && *piIndex < m_apNode.size()-2)
        fCurDistance += m_apNode[(*piIndex)++].fDistance;

    // calculate relative time between the nodes
    if(pfRelative)
    {
        *pfRelative = fDistance - fCurDistance;
        *pfRelative /= m_apNode[*piIndex].fDistance;
    }
}