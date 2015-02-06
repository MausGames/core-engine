//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

coreLookup<GLenum, GLuint> coreDataBuffer::s_aiBound; // = 0;


// ****************************************************************
// create buffer storage
void coreDataBuffer::Create(const GLenum& iTarget, const coreUint& iSize, const void* pData, const coreDataBufferStorage& iStorageType)
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

    if(CONTAINS_VALUE(m_iStorageType, CORE_DATABUFFER_STORAGE_STREAM))
    {
        // always allocate normal when streaming
        glBufferData(m_iTarget, m_iSize, pData, GL_STREAM_DRAW);
    }
    else if(CORE_GL_SUPPORT(ARB_buffer_storage))
    {
        // set storage flags
        GLenum iFlags = 0;
        switch(m_iStorageType & 0xFF)
        {
        case CORE_DATABUFFER_STORAGE_PERSISTENT: ADD_VALUE(iFlags, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT)
        case CORE_DATABUFFER_STORAGE_DYNAMIC:    ADD_VALUE(iFlags, GL_MAP_WRITE_BIT)
        default: break;
        }

        // allocate immutable buffer memory
        glBufferStorage(m_iTarget, m_iSize, pData, iFlags);

        // map persistent mapped buffer
        if(CONTAINS_VALUE(m_iStorageType, CORE_DATABUFFER_STORAGE_PERSISTENT))
            m_pPersistentBuffer = s_cast<coreByte*>(glMapBufferRange(m_iTarget, 0, m_iSize, iFlags | GL_MAP_INVALIDATE_BUFFER_BIT));
    }
    else
    {
        // allocate normal buffer memory
        glBufferData(m_iTarget, m_iSize, pData, this->IsWritable() ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    // create sync object
    if(CONTAINS_VALUE(m_iStorageType, CORE_DATABUFFER_STORAGE_FENCED))
        m_pSync = new coreSync();
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
        this->Unmap(r_cast<coreByte*>(~0ul));
    }

    // delete buffer
    glDeleteBuffers(1, &m_iDataBuffer);

    // delete sync object
    SAFE_DELETE(m_pSync)

    // reset properties
    m_iDataBuffer  = 0;
    m_iStorageType = CORE_DATABUFFER_STORAGE_STATIC;
    m_iTarget      = 0;
    m_iSize        = 0;
}


// ****************************************************************
// clear content of the data buffer object
void coreDataBuffer::Clear(const GLenum& iInternal, const GLenum& iFormat, const GLenum& iType, const void* pData)
{
    ASSERT(m_iDataBuffer && this->IsWritable())

    // clear the whole buffer
    if(CORE_GL_SUPPORT(ARB_clear_buffer_object))
    {
        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // clear content directly
            glClearNamedBufferData(m_iDataBuffer, iInternal, iFormat, iType, pData);
        }
        else
        {
            // bind and clear content
            this->Bind();
            glClearBufferData(m_iTarget, iInternal, iFormat, iType, pData);
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
: m_iVertexSize (0)
{
}


// ****************************************************************
// create buffer storage
void coreVertexBuffer::Create(const coreUint& iNumVertices, const coreByte& iVertexSize, const void* pVertexData, const coreDataBufferStorage& iStorageType)
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
    m_iVertexSize = 0;
    m_aAttribute.clear();
}


// ****************************************************************
// define vertex attribute array
void coreVertexBuffer::DefineAttribute(const int& iLocation, const coreByte& iComponents, const GLenum& iType, const bool& bInteger, const coreByte& iOffset)
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
void coreVertexBuffer::Activate(const coreByte& iBinding)
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
                const bool bNormalized = ((it->iType >= GL_BYTE) && (it->iType <= GL_UNSIGNED_INT)) || (it->iType == GL_INT_2_10_10_10_REV) || (it->iType == GL_UNSIGNED_INT_2_10_10_10_REV);
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
                const bool bNormalized = ((it->iType >= GL_BYTE) && (it->iType <= GL_UNSIGNED_INT)) || (it->iType == GL_INT_2_10_10_10_REV) || (it->iType == GL_UNSIGNED_INT_2_10_10_10_REV);
                glVertexAttribPointer(it->iLocation, it->iComponents, it->iType, bNormalized, m_iVertexSize, I_TO_P(it->iOffset));
            }
        }
    }
}