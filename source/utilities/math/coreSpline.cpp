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
coreSpline::coreSpline()noexcept
: m_fMaxDistance (0.0f)
{
    // reserve memory for nodes
    m_apNode.reserve(32);
}

coreSpline::coreSpline(const coreSpline& c)noexcept
: m_apNode       (c.m_apNode)
, m_fMaxDistance (c.m_fMaxDistance)
{
}

coreSpline::coreSpline(coreSpline&& m)noexcept
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
// assignment operator
coreSpline& coreSpline::operator = (coreSpline o)noexcept
{
    swap(*this, o);
    return *this;
}

void swap(coreSpline& a, coreSpline& b)noexcept
{
    using std::swap;
    swap(a.m_apNode,       b.m_apNode);
    swap(a.m_fMaxDistance, b.m_fMaxDistance);
}


// ****************************************************************
// add node to spline
void coreSpline::AddNode(const coreVector3& vPosition, const coreVector3& vTangent)
{
    ASSERT(vTangent.IsNormalized())

    // create new node
    coreNode NewNode;
    NewNode.vPosition = vPosition;
    NewNode.vTangent  = vTangent;

    // edit last node and increase max distance
    if(!m_apNode.empty())
    {
        coreNode& LastNode = m_apNode.back();

        LastNode.fDistance = (NewNode.vPosition - LastNode.vPosition).Length();
        m_fMaxDistance    += LastNode.fDistance;
    }

    // add new node
    m_apNode.push_back(NewNode);
}


// ****************************************************************
// remove node from spline
void coreSpline::DeleteNode(const coreUint& iIndex)
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

    return coreSpline::GetPosition(fRelative, m_apNode[iIndex].vPosition, m_apNode[iIndex+1].vPosition, vStartVel, vEndVel);
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

    return coreSpline::GetDirection(fRelative, m_apNode[iIndex].vPosition, m_apNode[iIndex+1].vPosition, vStartVel, vEndVel).Normalize();
}


// ****************************************************************
// get relative node and time
void coreSpline::GetRelative(const float& fTime, coreUint* piIndex, float* pfRelative)const
{
    const float fDistance = this->GetDistance(fTime);
    float fCurDistance = 0.0f;

    // search relevant node
    *piIndex = 0;
    while((fCurDistance + m_apNode[*piIndex].fDistance < fDistance) && (*piIndex < m_apNode.size()-2))
        fCurDistance += m_apNode[(*piIndex)++].fDistance;

    // calculate relative time between the nodes
    if(pfRelative)
    {
        *pfRelative  = fDistance - fCurDistance;
        *pfRelative *= RCP(m_apNode[*piIndex].fDistance);
    }
}