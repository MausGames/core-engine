//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

coreLookup<GLenum, GLuint> coreDataBuffer::s_aiBound = {};


// ****************************************************************
// destructor
coreDataBuffer::~coreDataBuffer()
{
    // delete buffer storage
    this->Delete();
}


// ****************************************************************
// assignment operations
coreDataBuffer& coreDataBuffer::operator = (coreDataBuffer o)noexcept
{
    std::swap(m_iDataBuffer,       o.m_iDataBuffer);
    std::swap(m_iStorageType,      o.m_iStorageType);
    std::swap(m_iTarget,           o.m_iTarget);
    std::swap(m_iSize,             o.m_iSize);
    std::swap(m_pPersistentBuffer, o.m_pPersistentBuffer);
    std::swap(m_iMapOffset,        o.m_iMapOffset);
    std::swap(m_iMapLength,        o.m_iMapLength);
    std::swap(m_Sync,              o.m_Sync);
    return *this;
}


// ****************************************************************
// create buffer storage
void coreDataBuffer::Create(const GLenum iTarget, const coreUint32 iSize, const void* pData, const coreDataBufferStorage iStorageType)
{
    WARN_IF(m_iDataBuffer) this->Delete();
    ASSERT(iSize)

    // save properties
    m_iStorageType = iStorageType;
    m_iTarget      = iTarget;
    m_iSize        = iSize;

    // generate buffer
    glGenBuffers(1, &m_iDataBuffer);
    glBindBuffer(m_iTarget, m_iDataBuffer);
    s_aiBound[m_iTarget] = m_iDataBuffer;

    if(CONTAINS_FLAG(m_iStorageType, CORE_DATABUFFER_STORAGE_STREAM))
    {
        // always allocate normal when streaming
        glBufferData(m_iTarget, m_iSize, pData, GL_STREAM_DRAW);
    }
    else if(CORE_GL_SUPPORT(ARB_buffer_storage))
    {
        // set storage flags
        GLenum iFlags = 0u;
        switch(m_iStorageType & 0xFFu)
        {
        case CORE_DATABUFFER_STORAGE_PERSISTENT: ADD_FLAG(iFlags, GL_MAP_PERSISTENT_BIT)
        case CORE_DATABUFFER_STORAGE_DYNAMIC:    ADD_FLAG(iFlags, GL_MAP_WRITE_BIT)
        default: break;
        }

        // allocate immutable buffer memory
        glBufferStorage(m_iTarget, m_iSize, pData, iFlags);

        // map persistent mapped buffer
        if(CONTAINS_FLAG(m_iStorageType, CORE_DATABUFFER_STORAGE_PERSISTENT))
            m_pPersistentBuffer = s_cast<coreByte*>(glMapBufferRange(m_iTarget, 0, m_iSize, iFlags | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT));
    }
    else
    {
        // allocate normal buffer memory
        glBufferData(m_iTarget, m_iSize, pData, this->IsWritable() ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }
}


// ****************************************************************
// delete buffer storage
void coreDataBuffer::Delete()
{
    if(!m_iDataBuffer) return;

    // unmap persistent mapped buffer
    if(m_pPersistentBuffer)
    {
        m_pPersistentBuffer = NULL;
        this->Unmap(r_cast<coreByte*>(~0x00));
    }

    // delete buffer
    glDeleteBuffers(1, &m_iDataBuffer);

    // delete sync object
    m_Sync.Delete();

    // reset properties
    m_iDataBuffer  = 0u;
    m_iStorageType = CORE_DATABUFFER_STORAGE_STATIC;
    m_iTarget      = 0u;
    m_iSize        = 0u;
}


// ****************************************************************
// clear content of the data buffer object
void coreDataBuffer::Clear(const coreTextureSpec& oTextureSpec, const void* pData)
{
    ASSERT(m_iDataBuffer && this->IsWritable())

    if(CORE_GL_SUPPORT(ARB_clear_buffer_object))
    {
        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // clear content directly (new)
            glClearNamedBufferData(m_iDataBuffer, oTextureSpec.iInternal, oTextureSpec.iFormat, oTextureSpec.iType, pData);
        }
        else if(CORE_GL_SUPPORT(EXT_direct_state_access))
        {
            // clear content directly (old)
            glClearNamedBufferDataEXT(m_iDataBuffer, oTextureSpec.iInternal, oTextureSpec.iFormat, oTextureSpec.iType, pData);
        }
        else
        {
            // bind and clear content
            this->Bind();
            glClearBufferData(m_iTarget, oTextureSpec.iInternal, oTextureSpec.iFormat, oTextureSpec.iType, pData);
        }
    }
}


// ****************************************************************
// invalidate content of the data buffer object
void coreDataBuffer::Invalidate()
{
    ASSERT(m_iDataBuffer && this->IsWritable())

    // invalidate the whole buffer
    if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
        glInvalidateBufferData(m_iDataBuffer);
}


// ****************************************************************
// constructor
coreVertexBuffer::coreVertexBuffer()noexcept
: coreDataBuffer ()
, m_iVertexSize  (0u)
, m_aAttribute   {}
{
}

coreVertexBuffer::coreVertexBuffer(coreVertexBuffer&& m)noexcept
: coreDataBuffer (std::move(m))
, m_iVertexSize  (m.m_iVertexSize)
, m_aAttribute   (std::move(m.m_aAttribute))
{
}


// ****************************************************************
// destructor
coreVertexBuffer::~coreVertexBuffer()
{
    // delete buffer storage
    this->Delete();
}


// ****************************************************************
// assignment operations
coreVertexBuffer& coreVertexBuffer::operator = (coreVertexBuffer o)noexcept
{
    std::swap(s_cast<coreDataBuffer&>(*this), s_cast<coreDataBuffer&>(o));
    std::swap(m_iVertexSize,                  o.m_iVertexSize);
    std::swap(m_aAttribute,                   o.m_aAttribute);
    return *this;
}


// ****************************************************************
// create buffer storage
void coreVertexBuffer::Create(const coreUint32 iNumVertices, const coreUint8 iVertexSize, const void* pVertexData, const coreDataBufferStorage iStorageType)
{
    // create buffer storage
    coreDataBuffer::Create(GL_ARRAY_BUFFER, iNumVertices*iVertexSize, pVertexData, iStorageType);

    // save properties
    m_iVertexSize = iVertexSize;
}


// ****************************************************************
// delete buffer storage
void coreVertexBuffer::Delete()
{
    if(!this->GetDataBuffer()) return;

    // delete buffer storage
    coreDataBuffer::Delete();

    // reset properties
    m_iVertexSize = 0u;
    m_aAttribute.clear();
}


// ****************************************************************
// define vertex attribute array
void coreVertexBuffer::DefineAttribute(const coreUint8 iLocation, const coreUint8 iComponents, const GLenum iType, const coreBool bInteger, const coreUint8 iOffset)
{
    ASSERT(this->GetDataBuffer())

#if defined(_CORE_DEBUG_)

    // check for duplicate vertex attribute arrays
    FOR_EACH(it, m_aAttribute)
        ASSERT(it->iLocation != iLocation)

#endif

    // create new vertex attribute array definition
    coreAttribute oNewAttribute;
    oNewAttribute.iLocation   = iLocation;
    oNewAttribute.iComponents = iComponents;
    oNewAttribute.iType       = iType;
    oNewAttribute.bInteger    = bInteger;
    oNewAttribute.iOffset     = iOffset;

    // add definition to list
    m_aAttribute.push_back(oNewAttribute);
}


// ****************************************************************
// activate the vertex structure
void coreVertexBuffer::Activate(const coreUint8 iBinding)
{
    ASSERT(this->GetDataBuffer() && !m_aAttribute.empty())

    if(CORE_GL_SUPPORT(ARB_vertex_attrib_binding))
    {
        // bind the vertex buffer
        glBindVertexBuffer(iBinding, this->GetDataBuffer(), 0, m_iVertexSize);

        FOR_EACH(it, m_aAttribute)
        {
            // enable each defined vertex attribute array
            glEnableVertexAttribArray(it->iLocation);
            glVertexAttribBinding(it->iLocation, iBinding);

            // specify the vertex format
            if(it->bInteger) glVertexAttribIFormat(it->iLocation, it->iComponents, it->iType, it->iOffset);
            else
            {
                const coreBool bNormalized = ((it->iType >= GL_BYTE) && (it->iType <= GL_UNSIGNED_INT)) || (it->iType == GL_INT_2_10_10_10_REV) || (it->iType == GL_UNSIGNED_INT_2_10_10_10_REV);
                glVertexAttribFormat(it->iLocation, it->iComponents, it->iType, bNormalized, it->iOffset);
            }
        }
    }
    else
    {
        // bind the vertex buffer
        this->Bind();

        FOR_EACH(it, m_aAttribute)
        {
            // enable each defined vertex attribute array
            glEnableVertexAttribArray(it->iLocation);

            // specify the vertex format
            if(it->bInteger) glVertexAttribIPointer(it->iLocation, it->iComponents, it->iType, m_iVertexSize, I_TO_P(it->iOffset));
            else
            {
                const coreBool bNormalized = ((it->iType >= GL_BYTE) && (it->iType <= GL_UNSIGNED_INT)) || (it->iType == GL_INT_2_10_10_10_REV) || (it->iType == GL_UNSIGNED_INT_2_10_10_10_REV);
                glVertexAttribPointer(it->iLocation, it->iComponents, it->iType, bNormalized, m_iVertexSize, I_TO_P(it->iOffset));
            }
        }
    }
}

void coreVertexBuffer::ActivateDivided(const coreUint8 iBinding, const coreUint8 iDivisor)
{
    // activate the vertex structure
    this->Activate(iBinding);

    if(CORE_GL_SUPPORT(ARB_instanced_arrays))
    {
        if(CORE_GL_SUPPORT(ARB_vertex_attrib_binding))
        {
            // enable array division per binding
            glVertexBindingDivisor(iBinding, iDivisor);
        }
        else
        {
            // enable array division per attribute
            FOR_EACH(it, m_aAttribute) glVertexAttribDivisor(it->iLocation, iDivisor);
        }
    }
}