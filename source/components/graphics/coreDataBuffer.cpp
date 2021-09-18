///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

coreMap<GLenum, GLuint> coreDataBuffer::s_aiBound = {};


// ****************************************************************
/* destructor */
coreDataBuffer::~coreDataBuffer()
{
    // delete buffer storage
    this->Delete();
}


// ****************************************************************
/* assignment operations */
coreDataBuffer& coreDataBuffer::operator = (coreDataBuffer&& m)noexcept
{
    // swap properties
    std::swap(m_iIdentifier,       m.m_iIdentifier);
    std::swap(m_eStorageType,      m.m_eStorageType);
    std::swap(m_iTarget,           m.m_iTarget);
    std::swap(m_iSize,             m.m_iSize);
    std::swap(m_iFallbackSize,     m.m_iFallbackSize);
    std::swap(m_pPersistentBuffer, m.m_pPersistentBuffer);
    std::swap(m_iMapOffset,        m.m_iMapOffset);
    std::swap(m_iMapLength,        m.m_iMapLength);
    std::swap(m_Sync,              m.m_Sync);

    return *this;
}


// ****************************************************************
/* create buffer storage */
void coreDataBuffer::Create(const GLenum iTarget, const coreUint32 iSize, const void* pData, const coreDataBufferStorage eStorageType)
{
    WARN_IF(m_iIdentifier) this->Delete();
    ASSERT(iSize)

    // save properties
    m_eStorageType = eStorageType;
    m_iTarget      = iTarget;
    m_iSize        = iSize;

    // generate buffer
    coreGenBuffers(1u, &m_iIdentifier);
    glBindBuffer(m_iTarget, m_iIdentifier);
    s_aiBound[m_iTarget] = m_iIdentifier;

    if(CORE_GL_SUPPORT(ARB_buffer_storage))
    {
        if(HAS_FLAG(m_eStorageType, CORE_DATABUFFER_STORAGE_STATIC))
        {
            // allocate static immutable buffer memory
            glBufferStorage(m_iTarget, m_iSize, pData, 0u);
        }
        else if(HAS_FLAG(m_eStorageType, CORE_DATABUFFER_STORAGE_DYNAMIC))
        {
            if(CORE_GL_SUPPORT(ARB_map_buffer_range))
            {
                // allocate dynamic immutable buffer memory
                glBufferStorage(m_iTarget, m_iSize, pData, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);

                // map persistent mapped buffer
                m_pPersistentBuffer = s_cast<coreByte*>(glMapBufferRange(m_iTarget, 0, m_iSize, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT));
            }
            else
            {
                // allocate dynamic immutable buffer memory (without persistent mapping)
                glBufferStorage(m_iTarget, m_iSize, pData, GL_DYNAMIC_STORAGE_BIT);
            }
        }
        else
        {
            // allocate temporary immutable buffer memory
            glBufferStorage(m_iTarget, m_iSize, pData, GL_CLIENT_STORAGE_BIT);
        }
    }
    else
    {
        if(HAS_FLAG(m_eStorageType, CORE_DATABUFFER_STORAGE_STATIC))
        {
            // allocate static mutable buffer memory
            glBufferData(m_iTarget, m_iSize, pData, GL_STATIC_DRAW);
        }
        else if(HAS_FLAG(m_eStorageType, CORE_DATABUFFER_STORAGE_DYNAMIC))
        {
            // allocate dynamic mutable buffer memory
            glBufferData(m_iTarget, m_iSize, pData, GL_DYNAMIC_DRAW);
        }
        else
        {
            // allocate temporary mutable buffer memory
            glBufferData(m_iTarget, m_iSize, pData, GL_STREAM_DRAW);
        }
    }

    if(CORE_GL_SUPPORT(NV_shader_buffer_load))
    {
        // make sure static buffers are in GPU memory
        if(HAS_FLAG(m_eStorageType, CORE_DATABUFFER_STORAGE_STATIC))
            glMakeBufferResidentNV(m_iTarget, GL_READ_ONLY);
    }
}


// ****************************************************************
/* delete buffer storage */
void coreDataBuffer::Delete()
{
    if(!m_iIdentifier) return;

    // delete fallback memory
    if(m_iFallbackSize)
    {
        DYNAMIC_DELETE(m_pPersistentBuffer)
        m_iFallbackSize = 0u;
    }

    // unmap persistent mapped buffer
    if(m_pPersistentBuffer)
    {
        m_pPersistentBuffer = NULL;
        this->Unmap(r_cast<coreByte*>(-1));
    }

    // delete buffer
    glDeleteBuffers(1, &m_iIdentifier);

    // delete sync object
    m_Sync.Delete();

    // reset properties
    m_iIdentifier  = 0u;
    m_eStorageType = CORE_DATABUFFER_STORAGE_STATIC;
    m_iTarget      = 0u;
    m_iSize        = 0u;
}


// ****************************************************************
/* map buffer memory for writing operations */
RETURN_RESTRICT coreByte* coreDataBuffer::Map(const coreUint32 iOffset, const coreUint32 iLength, const coreDataBufferMap eMapType)
{
    ASSERT(m_iIdentifier && this->IsWritable() && ((iOffset + iLength) <= m_iSize))

    // save mapping attributes
    m_iMapOffset = iOffset;
    m_iMapLength = iLength;

    if(CORE_GL_SUPPORT(ARB_map_buffer_range))
    {
        // check for sync object status
        m_Sync.Check(CORE_SYNC_WAIT_FOREVER, CORE_SYNC_CHECK_NORMAL);

        // return persistent mapped buffer
        if(m_pPersistentBuffer) return (m_pPersistentBuffer + iOffset);

        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // map buffer memory directly (new)
            return s_cast<coreByte*>(glMapNamedBufferRange(m_iIdentifier, iOffset, iLength, GL_MAP_WRITE_BIT | eMapType));
        }
        else if(CORE_GL_SUPPORT(EXT_direct_state_access))
        {
            // map buffer memory directly (old)
            return s_cast<coreByte*>(glMapNamedBufferRangeEXT(m_iIdentifier, iOffset, iLength, GL_MAP_WRITE_BIT | eMapType));
        }
        else
        {
            // bind and map buffer memory
            this->Bind();
            return s_cast<coreByte*>(glMapBufferRange(m_iTarget, iOffset, iLength, GL_MAP_WRITE_BIT | eMapType));
        }
    }
    else
    {
        // create fallback memory
        if(m_iFallbackSize < iLength)
        {
            DYNAMIC_RESIZE(m_pPersistentBuffer, iLength)
            m_iFallbackSize = iLength;
        }
        return m_pPersistentBuffer;
    }
}


// ****************************************************************
/* unmap buffer memory */
void coreDataBuffer::Unmap(const coreByte* pPointer)
{
    ASSERT(pPointer)

    if(CORE_GL_SUPPORT(ARB_map_buffer_range))
    {
        if(m_pPersistentBuffer)
        {
            if(CORE_GL_SUPPORT(ARB_direct_state_access))
            {
                // flush persistent mapped buffer directly (new)
                glFlushMappedNamedBufferRange(m_iIdentifier, m_iMapOffset, m_iMapLength);
            }
            else if(CORE_GL_SUPPORT(EXT_direct_state_access))
            {
                // flush persistent mapped buffer directly (old)
                glFlushMappedNamedBufferRangeEXT(m_iIdentifier, m_iMapOffset, m_iMapLength);
            }
            else
            {
                // bind and flush persistent mapped buffer
                this->Bind();
                glFlushMappedBufferRange(m_iTarget, m_iMapOffset, m_iMapLength);
            }
        }
        else
        {
            if(CORE_GL_SUPPORT(ARB_direct_state_access))
            {
                // unmap buffer memory directly (new)
                glUnmapNamedBuffer(m_iIdentifier);
            }
            else if(CORE_GL_SUPPORT(EXT_direct_state_access))
            {
                // unmap buffer memory directly (old)
                glUnmapNamedBufferEXT(m_iIdentifier);
            }
            else
            {
                // bind and unmap buffer memory
                this->Bind();
                glUnmapBuffer(m_iTarget);
            }
        }
    }
    else
    {
        // send new data to the data buffer
        this->Bind();
        glBufferSubData(m_iTarget, m_iMapOffset, m_iMapLength, pPointer);
    }

    // reset mapping attributes
    m_iMapOffset = 0u;
    m_iMapLength = 0u;
}


// ****************************************************************
/* copy content of the data buffer object */
coreStatus coreDataBuffer::Copy(const coreUint32 iReadOffset, const coreUint32 iWriteOffset, const coreUint32 iLength, coreDataBuffer* OUTPUT pDestination)const
{
    ASSERT(m_iIdentifier && ((iReadOffset + iLength) <= m_iSize) && ((iWriteOffset + iLength) <= pDestination->GetSize()))

    if(CORE_GL_SUPPORT(ARB_copy_buffer))
    {
        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // copy content directly (new)
            glCopyNamedBufferSubData(m_iIdentifier, pDestination->GetIdentifier(), iReadOffset, iWriteOffset, iLength);
        }
        else if(CORE_GL_SUPPORT(EXT_direct_state_access))
        {
            // copy content directly (old)
            glNamedCopyBufferSubDataEXT(m_iIdentifier, pDestination->GetIdentifier(), iReadOffset, iWriteOffset, iLength);
        }
        else
        {
            // bind and copy content
            coreDataBuffer::Bind(GL_COPY_READ_BUFFER,  m_iIdentifier);
            coreDataBuffer::Bind(GL_COPY_WRITE_BUFFER, pDestination->GetIdentifier());
            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, iReadOffset, iWriteOffset, iLength);
        }

        return CORE_OK;
    }

    return CORE_ERROR_SUPPORT;
}


// ****************************************************************
/* clear content of the data buffer object */
coreStatus coreDataBuffer::Clear(const coreTextureSpec& oTextureSpec, const void* pData)
{
    ASSERT(m_iIdentifier && this->IsWritable())

    if(CORE_GL_SUPPORT(ARB_clear_buffer_object))
    {
        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // clear content directly (new)
            glClearNamedBufferData(m_iIdentifier, oTextureSpec.iInternal, oTextureSpec.iFormat, oTextureSpec.iType, pData);
        }
        else if(CORE_GL_SUPPORT(EXT_direct_state_access))
        {
            // clear content directly (old)
            glClearNamedBufferDataEXT(m_iIdentifier, oTextureSpec.iInternal, oTextureSpec.iFormat, oTextureSpec.iType, pData);
        }
        else
        {
            // bind and clear content
            this->Bind();
            glClearBufferData(m_iTarget, oTextureSpec.iInternal, oTextureSpec.iFormat, oTextureSpec.iType, pData);
        }

        return CORE_OK;
    }

    return CORE_ERROR_SUPPORT;
}


// ****************************************************************
/* invalidate content of the data buffer object */
coreStatus coreDataBuffer::Invalidate()
{
    ASSERT(m_iIdentifier && this->IsWritable())

    if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
    {
        // invalidate the whole buffer
        glInvalidateBufferData(m_iIdentifier);

        return CORE_OK;
    }

    return CORE_ERROR_SUPPORT;
}


// ****************************************************************
/* constructor */
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
/* destructor */
coreVertexBuffer::~coreVertexBuffer()
{
    // delete buffer storage
    this->Delete();
}


// ****************************************************************
/* assignment operations */
coreVertexBuffer& coreVertexBuffer::operator = (coreVertexBuffer&& m)noexcept
{
    // swap properties
    this->coreDataBuffer::operator = (std::move(m));
    std::swap(m_iVertexSize, m.m_iVertexSize);
    std::swap(m_aAttribute,  m.m_aAttribute);

    return *this;
}


// ****************************************************************
/* create buffer storage */
void coreVertexBuffer::Create(const coreUint32 iNumVertices, const coreUint8 iVertexSize, const void* pVertexData, const coreDataBufferStorage eStorageType)
{
    // create buffer storage
    this->coreDataBuffer::Create(GL_ARRAY_BUFFER, iNumVertices * iVertexSize, pVertexData, eStorageType);

    // save properties
    m_iVertexSize = iVertexSize;
}


// ****************************************************************
/* delete buffer storage */
void coreVertexBuffer::Delete()
{
    if(!this->GetIdentifier()) return;

    // delete buffer storage
    this->coreDataBuffer::Delete();

    // reset properties
    m_iVertexSize = 0u;
    m_aAttribute.clear();
}


// ****************************************************************
/* define vertex attribute array */
void coreVertexBuffer::DefineAttribute(const coreUint8 iLocation, const coreUint8 iComponents, const GLenum iType, const coreBool bInteger, const coreUint8 iOffset)
{
    ASSERT(this->GetIdentifier())

#if defined(_CORE_DEBUG_)

    // check for duplicate vertex attribute arrays
    FOR_EACH(it, m_aAttribute)
        ASSERT(it->iLocation != iLocation)

#endif

    // create new vertex attribute array definition
    coreAttribute oNewAttribute;
    oNewAttribute.iType       = iType;
    oNewAttribute.iLocation   = iLocation;
    oNewAttribute.iComponents = iComponents;
    oNewAttribute.bInteger    = bInteger;
    oNewAttribute.iOffset     = iOffset;

    // add definition to list
    m_aAttribute.push_back(oNewAttribute);
}


// ****************************************************************
/* activate the vertex structure */
void coreVertexBuffer::Activate(const coreUint8 iBinding)
{
    ASSERT(this->GetIdentifier() && !m_aAttribute.empty())

    if(CORE_GL_SUPPORT(ARB_vertex_attrib_binding))
    {
        // bind the vertex buffer
        glBindVertexBuffer(iBinding, this->GetIdentifier(), 0, m_iVertexSize);

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
            if(it->bInteger && CORE_GL_SUPPORT(EXT_gpu_shader4)) glVertexAttribIPointer(it->iLocation, it->iComponents, it->iType, m_iVertexSize, I_TO_P(it->iOffset));
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