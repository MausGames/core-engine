///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
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

    if(HAS_FLAG(m_eStorageType, CORE_DATABUFFER_STORAGE_STATIC))
    {
        ASSERT(pData)

        if(CORE_GL_SUPPORT(ARB_buffer_storage))
        {
            // allocate static immutable buffer memory
            glBufferStorage(m_iTarget, m_iSize, pData, 0u);
        }
        else
        {
            // allocate static mutable buffer memory
            glBufferData(m_iTarget, m_iSize, pData, GL_STATIC_DRAW);
        }
    }
    else if(HAS_FLAG(m_eStorageType, CORE_DATABUFFER_STORAGE_DYNAMIC))
    {
        if(CORE_GL_SUPPORT(ARB_buffer_storage) && CORE_GL_SUPPORT(ARB_map_buffer_range))
        {
            // allocate dynamic immutable buffer memory
            glBufferStorage(m_iTarget, m_iSize, pData, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);

            // map persistent mapped buffer
            m_pPersistentBuffer = s_cast<coreByte*>(glMapBufferRange(m_iTarget, 0, m_iSize, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT));
        }
        else
        {
            // allocate dynamic mutable buffer memory
            glBufferData(m_iTarget, m_iSize, pData, GL_DYNAMIC_DRAW);
        }
    }
    else if(HAS_FLAG(m_eStorageType, CORE_DATABUFFER_STORAGE_STREAM))
    {
        ASSERT(pData)

        if(CORE_GL_SUPPORT(ARB_buffer_storage))
        {
            // allocate temporary immutable buffer memory
            glBufferStorage(m_iTarget, m_iSize, pData, GL_CLIENT_STORAGE_BIT);
        }
        else
        {
            // allocate temporary mutable buffer memory
            glBufferData(m_iTarget, m_iSize, pData, GL_STREAM_DRAW);
        }
    }
    else if(HAS_FLAG(m_eStorageType, CORE_DATABUFFER_STORAGE_READ))
    {
        if(CORE_GL_SUPPORT(ARB_buffer_storage))
        {
            // allocate readable immutable buffer memory
            glBufferStorage(m_iTarget, m_iSize, pData, GL_CLIENT_STORAGE_BIT | GL_MAP_READ_BIT);
        }
        else
        {
            // allocate readable mutable buffer memory
            glBufferData(m_iTarget, m_iSize, pData, GL_STREAM_READ);
        }
    }
    else WARN_IF(true) {}
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
        this->Unmap();
    }

    // delete buffer
    coreDelBuffers(1u, &m_iIdentifier);

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
RETURN_RESTRICT coreByte* coreDataBuffer::MapWrite(const coreUint32 iOffset, const coreUint32 iLength, const coreDataBufferMap eMapType)
{
    ASSERT(m_iIdentifier && this->IsWritable() && !this->IsMapped() && iLength && (iOffset + iLength <= m_iSize))

    // save mapping attributes
    m_iMapOffset = iOffset;
    m_iMapLength = iLength;

    if(CORE_GL_SUPPORT(ARB_map_buffer_range))
    {
        // check for sync object status
        WARN_IF(m_Sync.Check(CORE_SYNC_WAIT_FOREVER) == CORE_BUSY) {}

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
        if(eMapType == CORE_DATABUFFER_MAP_INVALIDATE_ALL)
        {
            if(CORE_GL_SUPPORT(ARB_direct_state_access))
            {
                // manually orphan buffer memory directly (new)
                glNamedBufferData(m_iIdentifier, m_iSize, NULL, GL_DYNAMIC_DRAW);
            }
            else if(CORE_GL_SUPPORT(EXT_direct_state_access))
            {
                // manually orphan buffer memory directly (old)
                glNamedBufferDataEXT(m_iIdentifier, m_iSize, NULL, GL_DYNAMIC_DRAW);
            }
            else
            {
                // bind and manually orphan buffer memory
                this->Bind();
                glBufferData(m_iTarget, m_iSize, NULL, GL_DYNAMIC_DRAW);
            }
        }

        if(m_iFallbackSize < iLength)
        {
            // create fallback memory
            DYNAMIC_RESIZE(m_pPersistentBuffer, iLength)
            m_iFallbackSize = iLength;
        }

        return m_pPersistentBuffer;
    }
}


// ****************************************************************
/* map buffer memory for reading operations */
RETURN_RESTRICT coreByte* coreDataBuffer::MapRead(const coreUint32 iOffset, const coreUint32 iLength)
{
    ASSERT(m_iIdentifier && this->IsReadable() && !this->IsMapped() && iLength && (iOffset + iLength <= m_iSize))

    // save mapping attributes
    m_iMapOffset = iOffset;
    m_iMapLength = iLength;

    if(CORE_GL_SUPPORT(ARB_map_buffer_range))
    {
        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // map buffer memory directly (new)
            return s_cast<coreByte*>(glMapNamedBufferRange(m_iIdentifier, iOffset, iLength, GL_MAP_READ_BIT));
        }
        else if(CORE_GL_SUPPORT(EXT_direct_state_access))
        {
            // map buffer memory directly (old)
            return s_cast<coreByte*>(glMapNamedBufferRangeEXT(m_iIdentifier, iOffset, iLength, GL_MAP_READ_BIT));
        }
        else
        {
            // bind and map buffer memory
            this->Bind();
            return s_cast<coreByte*>(glMapBufferRange(m_iTarget, iOffset, iLength, GL_MAP_READ_BIT));
        }
    }
    else
    {
        if(m_iFallbackSize < iLength)
        {
            // create fallback memory
            DYNAMIC_RESIZE(m_pPersistentBuffer, iLength)
            m_iFallbackSize = iLength;
        }

        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // retrieve data from the data buffer directly (new)
            glGetNamedBufferSubData(m_iIdentifier, iOffset, iLength, m_pPersistentBuffer);
        }
        else if(CORE_GL_SUPPORT(EXT_direct_state_access))
        {
            // retrieve data from the data buffer directly (old)
            glGetNamedBufferSubDataEXT(m_iIdentifier, iOffset, iLength, m_pPersistentBuffer);
        }
        else
        {
            // bind and retrieve data from the data buffer
            this->Bind();
            glGetBufferSubData(m_iTarget, iOffset, iLength, m_pPersistentBuffer);
        }

        return m_pPersistentBuffer;
    }
}


// ****************************************************************
/* unmap buffer memory */
void coreDataBuffer::Unmap()
{
    ASSERT(m_iIdentifier)   // # no map check, to unmap persistent mapped buffers

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
                WARN_IF(!glUnmapNamedBuffer(m_iIdentifier)) {}
            }
            else if(CORE_GL_SUPPORT(EXT_direct_state_access))
            {
                // unmap buffer memory directly (old)
                WARN_IF(!glUnmapNamedBufferEXT(m_iIdentifier)) {}
            }
            else
            {
                // bind and unmap buffer memory
                this->Bind();
                WARN_IF(!glUnmapBuffer(m_iTarget)) {}
            }
        }
    }
    else
    {
        ASSERT(m_pPersistentBuffer)

        if(this->IsWritable())
        {
            if(CORE_GL_SUPPORT(ARB_direct_state_access))
            {
                // send new data to the data buffer directly (new)
                glNamedBufferSubData(m_iIdentifier, m_iMapOffset, m_iMapLength, m_pPersistentBuffer);
            }
            else if(CORE_GL_SUPPORT(EXT_direct_state_access))
            {
                // send new data to the data buffer directly (old)
                glNamedBufferSubDataEXT(m_iIdentifier, m_iMapOffset, m_iMapLength, m_pPersistentBuffer);
            }
            else
            {
                // bind and send new data to the data buffer
                this->Bind();
                glBufferSubData(m_iTarget, m_iMapOffset, m_iMapLength, m_pPersistentBuffer);
            }
        }
    }

    // reset mapping attributes
    m_iMapOffset = 0u;
    m_iMapLength = 0u;
}


// ****************************************************************
/* copy content of the data buffer object */
coreStatus coreDataBuffer::Copy(const coreUint32 iReadOffset, const coreUint32 iWriteOffset, const coreUint32 iLength, coreDataBuffer* OUTPUT pDestination)const
{
    ASSERT(m_iIdentifier && iLength && (iReadOffset + iLength <= m_iSize) && (iWriteOffset + iLength <= pDestination->GetSize()))

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

    // not possible during mapping
    if(this->IsPersistent() || this->IsMapped()) return CORE_INVALID_CALL;

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
, m_iNumVertices (0u)
, m_iVertexSize  (0u)
, m_aAttribute   {}
, m_aStream      {}
{
}

coreVertexBuffer::coreVertexBuffer(coreVertexBuffer&& m)noexcept
: coreDataBuffer (std::move(m))
, m_iNumVertices (m.m_iNumVertices)
, m_iVertexSize  (m.m_iVertexSize)
, m_aAttribute   (std::move(m.m_aAttribute))
, m_aStream      (std::move(m.m_aStream))
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
    std::swap(m_iNumVertices, m.m_iNumVertices);
    std::swap(m_iVertexSize,  m.m_iVertexSize);
    std::swap(m_aAttribute,   m.m_aAttribute);
    std::swap(m_aStream,      m.m_aStream);

    return *this;
}


// ****************************************************************
/* create buffer storage */
void coreVertexBuffer::Create(const coreUint32 iNumVertices, const coreUint8 iVertexSize, const void* pVertexData, const coreDataBufferStorage eStorageType)
{
    // create buffer storage
    this->coreDataBuffer::Create(GL_ARRAY_BUFFER, iNumVertices * iVertexSize, pVertexData, eStorageType);

    // save properties
    m_iNumVertices = iNumVertices;
    m_iVertexSize  = iVertexSize;
}


// ****************************************************************
/* delete buffer storage */
void coreVertexBuffer::Delete()
{
    if(!this->GetIdentifier()) return;

    // delete buffer storage
    this->coreDataBuffer::Delete();

    // reset properties
    m_iNumVertices = 0u;
    m_iVertexSize  = 0u;
    m_aAttribute.clear();
    m_aStream   .clear();
}


// ****************************************************************
/* define vertex attribute array */
void coreVertexBuffer::DefineAttribute(const coreUint8 iLocation, const coreUint8 iComponents, const GLenum iType, const coreUint8 iSize, const coreBool bInteger, const coreUint8 iBufferOffset, const coreUint8 iVertexOffset)
{
    ASSERT(this->GetIdentifier() && (iLocation < CORE_VERTEXBUFFER_ATTRIBUTES))

#if defined(_CORE_DEBUG_)

    // check for duplicate vertex attribute arrays
    FOR_EACH(it, m_aAttribute)
    {
        ASSERT(it->iLocation != iLocation)
    }

#endif

    // create new vertex attribute array definition
    coreAttribute oNewAttribute;
    oNewAttribute.iType         = iType;
    oNewAttribute.iLocation     = iLocation;
    oNewAttribute.iComponents   = iComponents;
    oNewAttribute.bInteger      = bInteger;
    oNewAttribute.iBufferOffset = iBufferOffset;
    oNewAttribute.iVertexOffset = iVertexOffset;

    // add definition to list
    m_aAttribute.push_back(oNewAttribute);

    // accumulate vertex stream
    coreStream& oStream = m_aStream[iBufferOffset];
    oStream.iBinding = iLocation;   // use any attribute location as binding point index
    oStream.iStride += iSize;
}


// ****************************************************************
/* activate the vertex structure */
void coreVertexBuffer::Activate(const coreUint8 iDivisor)
{
    ASSERT(this->GetIdentifier() && !m_aAttribute.empty() && !m_aStream.empty())

    if(CORE_GL_SUPPORT(ARB_vertex_attrib_binding))
    {
        if((m_aStream.size() > 1u) && CORE_GL_SUPPORT(ARB_multi_bind))
        {
            GLuint   aiBuffer[CORE_VERTEXBUFFER_ATTRIBUTES] = {};
            GLintptr aiOffset[CORE_VERTEXBUFFER_ATTRIBUTES] = {};
            GLsizei  aiStride[CORE_VERTEXBUFFER_ATTRIBUTES] = {};

            coreUint8 iStart = UINT8_MAX;
            coreUint8 iEnd   = 0u;

            // loop through all vertex streams
            FOR_EACH(it, m_aStream)
            {
                const coreUint8 iBinding = it->iBinding;
                ASSERT(iBinding < CORE_VERTEXBUFFER_ATTRIBUTES)

                // insert vertex stream properties
                aiBuffer[iBinding] = this->GetIdentifier();
                aiOffset[iBinding] = (*m_aStream.get_key(it)) * m_iNumVertices;
                aiStride[iBinding] = it->iStride;

                // set range of binding point indices
                iStart = MIN(iStart, iBinding);
                iEnd   = MAX(iEnd,   iBinding);
            }

            // bind all at once
            ASSERT(iStart <= iEnd)
            glBindVertexBuffers(iStart, iEnd - iStart + 1u, aiBuffer + iStart, aiOffset + iStart, aiStride + iStart);
        }
        else
        {
            FOR_EACH(it, m_aStream)
            {
                // bind the vertex buffer
                glBindVertexBuffer(it->iBinding, this->GetIdentifier(), (*m_aStream.get_key(it)) * m_iNumVertices, it->iStride);
            }
        }

        FOR_EACH(it, m_aAttribute)
        {
            // enable each defined vertex attribute array
            glEnableVertexAttribArray(it->iLocation);
            glVertexAttribBinding(it->iLocation, m_aStream.at(it->iBufferOffset).iBinding);

            // specify the vertex format
            if(it->bInteger)
            {
                glVertexAttribIFormat(it->iLocation, it->iComponents, it->iType, it->iVertexOffset);
            }
            else
            {
                const coreBool bNormalized = ((it->iType >= GL_BYTE) && (it->iType <= GL_UNSIGNED_INT)) || (it->iType == GL_INT_2_10_10_10_REV) || (it->iType == GL_UNSIGNED_INT_2_10_10_10_REV);
                glVertexAttribFormat(it->iLocation, it->iComponents, it->iType, bNormalized, it->iVertexOffset);
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
            if(it->bInteger && CORE_GL_SUPPORT(EXT_gpu_shader4))
            {
                glVertexAttribIPointer(it->iLocation, it->iComponents, it->iType, m_aStream.at(it->iBufferOffset).iStride, I_TO_P(it->iBufferOffset * m_iNumVertices + it->iVertexOffset));
            }
            else
            {
                const coreBool bNormalized = (!it->bInteger) && (((it->iType >= GL_BYTE) && (it->iType <= GL_UNSIGNED_INT)) || (it->iType == GL_INT_2_10_10_10_REV) || (it->iType == GL_UNSIGNED_INT_2_10_10_10_REV));
                glVertexAttribPointer(it->iLocation, it->iComponents, it->iType, bNormalized, m_aStream.at(it->iBufferOffset).iStride, I_TO_P(it->iBufferOffset * m_iNumVertices + it->iVertexOffset));
            }
        }
    }

    if(iDivisor && CORE_GL_SUPPORT(ARB_instanced_arrays))
    {
        if(CORE_GL_SUPPORT(ARB_vertex_attrib_binding))
        {
            // enable array division per binding
            FOR_EACH(it, m_aStream) glVertexBindingDivisor(it->iBinding, iDivisor);
        }
        else
        {
            // enable array division per attribute
            FOR_EACH(it, m_aAttribute) glVertexAttribDivisor(it->iLocation, iDivisor);
        }
    }
}


// ****************************************************************
/* constructor */
coreUniformBuffer::coreUniformBuffer()noexcept
: coreDataBuffer ()
, m_iBinding     (0u)
, m_iRangeCount  (0u)
, m_iRangeSize   (0u)
, m_aSync        {}
{
}

coreUniformBuffer::coreUniformBuffer(coreUniformBuffer&& m)noexcept
: coreDataBuffer (std::move(m))
, m_iBinding     (m.m_iBinding)
, m_iRangeCount  (m.m_iRangeCount)
, m_iRangeSize   (m.m_iRangeSize)
, m_aSync        (std::move(m.m_aSync))
{
}


// ****************************************************************
/* destructor */
coreUniformBuffer::~coreUniformBuffer()
{
    // delete buffer storage
    this->Delete();
}


// ****************************************************************
/* assignment operations */
coreUniformBuffer& coreUniformBuffer::operator = (coreUniformBuffer&& m)noexcept
{
    // swap properties
    this->coreDataBuffer::operator = (std::move(m));
    std::swap(m_iBinding,    m.m_iBinding);
    std::swap(m_iRangeCount, m.m_iRangeCount);
    std::swap(m_iRangeSize,  m.m_iRangeSize);
    std::swap(m_aSync,       m.m_aSync);

    return *this;
}


// ****************************************************************
/* create buffer storage */
void coreUniformBuffer::Create(const coreUint8 iBinding, const coreUint8 iRangeCount, const coreUint32 iRangeSize)
{
    if(CORE_GL_SUPPORT(ARB_uniform_buffer_object))
    {
        // create buffer storage
        this->coreDataBuffer::Create(GL_UNIFORM_BUFFER, iRangeCount * coreMath::CeilAlign(iRangeSize, 256u), NULL, CORE_DATABUFFER_STORAGE_DYNAMIC);

        // create sync objects
        m_aSync.resize(iRangeCount);

        // save properties
        m_iBinding    = iBinding;
        m_iRangeCount = iRangeCount;
        m_iRangeSize  = iRangeSize;
    }
}


// ****************************************************************
/* delete buffer storage */
void coreUniformBuffer::Delete()
{
    if(!this->GetIdentifier()) return;

    // delete buffer storage
    this->coreDataBuffer::Delete();

    // delete sync objects
    m_aSync.clear();

    // reset properties
    m_iBinding    = 0u;
    m_iRangeCount = 0u;
    m_iRangeSize  = 0u;
}


// ****************************************************************
/* map and bind next buffer range */
RETURN_RESTRICT coreByte* coreUniformBuffer::MapWriteNext()
{
    ASSERT(this->GetIdentifier())

    // invalidate previous buffer range
    if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
    {
        if(!this->IsPersistent())
        {
            const coreUint32 iOldOffset = m_aSync.index() * coreMath::CeilAlign(m_iRangeSize, 256u);   // old
            glInvalidateBufferSubData(this->GetIdentifier(), iOldOffset, coreMath::CeilAlign(m_iRangeSize, 256u));
        }
    }

    // synchronize and switch to next sync object
    m_aSync.current().Create(CORE_SYNC_CREATE_NORMAL);
    m_aSync.next();
    WARN_IF(m_aSync.current().Check(CORE_SYNC_WAIT_FOREVER) == CORE_BUSY) {}

    // bind next buffer range
    const coreUint32 iNewOffset = m_aSync.index() * coreMath::CeilAlign(m_iRangeSize, 256u);   // new
    glBindBufferRange(GL_UNIFORM_BUFFER, m_iBinding, this->GetIdentifier(), iNewOffset, coreMath::CeilAlign(m_iRangeSize, 16u));

    // map buffer range
    return this->MapWrite(iNewOffset, m_iRangeSize, CORE_DATABUFFER_MAP_UNSYNCHRONIZED);
}