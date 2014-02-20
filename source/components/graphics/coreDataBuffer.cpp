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
// create buffer storage
void coreDataBuffer::Create(const GLenum& iTarget, const coreUint& iSize, const void* pData, const GLenum& iUsage)
{
    SDL_assert(!m_iDataBuffer);

    // save attributes
    m_iTarget  = iTarget;
    m_iSize    = iSize;
    m_bDynamic = (iUsage != GL_STATIC_DRAW && iUsage != GL_STATIC_READ && iUsage != GL_STATIC_COPY) ? true : false;

    // generate and bind buffer 
    glGenBuffers(1, &m_iDataBuffer);
    glBindBuffer(m_iTarget, m_iDataBuffer);

    // allocate buffer memory
    if(GLEW_ARB_buffer_storage) glBufferStorage(m_iTarget, m_iSize, pData, m_bDynamic ? GL_MAP_WRITE_BIT : 0);
                           else glBufferData(m_iTarget, m_iSize, pData, iUsage);
}


// ****************************************************************
// delete buffer storage
void coreDataBuffer::Delete()
{
    if(!m_iDataBuffer) return;
    
    // delete buffer
    glDeleteBuffers(1, &m_iDataBuffer); 
    
    // reset attributes
    m_iDataBuffer = 0;
    m_iTarget     = 0;
    m_iSize       = 0;
    m_bDynamic    = false;
}


// ****************************************************************
// map buffer memory for writing operations
coreByte* coreDataBuffer::Map(const coreUint& iOffset, const coreUint& iLength)
{
    SDL_assert(m_iDataBuffer && m_bDynamic && (iOffset+iLength <= m_iSize));

    // bind the data buffer
    this->Bind();
    
    if(GLEW_ARB_map_buffer_range)
    {
        // directly map buffer memory
        return s_cast<coreByte*>(glMapBufferRange(m_iTarget, iOffset, iLength, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT));
    }
    else
    {
        // create temporary memory
        coreByte* pPointer = new coreByte[iLength + sizeof(coreUint)*2];

        // add mapping attributes
        std::memcpy(pPointer,                    &iOffset, sizeof(coreUint));
        std::memcpy(pPointer + sizeof(coreUint), &iLength, sizeof(coreUint));

        return pPointer + sizeof(coreUint)*2;
    }
}


// ****************************************************************
// unmap buffer memory
void coreDataBuffer::Unmap(coreByte* pPointer)
{
    SDL_assert(pPointer);

    if(GLEW_ARB_map_buffer_range)
    {
        // directly unmap buffer memory
        glUnmapBuffer(m_iTarget);
    }
    else
    {
        // extract mapping attributes
        coreUint iOffset; std::memcpy(&iOffset, pPointer - sizeof(coreUint)*2, sizeof(coreUint));
        coreUint iLength; std::memcpy(&iLength, pPointer - sizeof(coreUint)*1, sizeof(coreUint));

        // send new data to the data buffer
        glBufferSubData(m_iTarget, iOffset, iLength, pPointer);

        // delete temporary memory
        pPointer -= sizeof(coreUint)*2;
        SAFE_DELETE_ARRAY(pPointer);
    }
}


// ****************************************************************
// constructor
coreVertexBuffer::coreVertexBuffer()noexcept
: m_iVertexSize (0)
{
    // reserve memory for vertex attribute arrays
    m_aAttribute.reserve(4);
}


// ****************************************************************
// create buffer storage
void coreVertexBuffer::Create(const coreUint& iNumVertices, const coreByte& iVertexSize, const void* pVertexData, const GLenum& iUsage)
{
    // create buffer storage
    this->coreDataBuffer::Create(GL_ARRAY_BUFFER, iNumVertices*iVertexSize, pVertexData, iUsage);

    // save attributes
    m_iVertexSize = iVertexSize; 
}


// ****************************************************************
// delete buffer storage
void coreVertexBuffer::Delete()
{
    if(!this->GetDataBuffer()) return;

    // delete buffer storage
    this->coreDataBuffer::Delete();

    // reset attributes
    m_iVertexSize = 0;
    m_aAttribute.clear();      
}


// ****************************************************************
// define vertex attribute array
void coreVertexBuffer::DefineAttribute(const int& iLocation, const coreByte& iComponents, const GLenum& iType, const coreByte& iOffset)
{
    SDL_assert(this->GetDataBuffer());

#if defined(_CORE_DEBUG_)

    // check for duplicate vertex attribute arrays
    FOR_EACH(it, m_aAttribute)
        SDL_assert(it->iLocation != iLocation);

#endif

    // create new vertex attribute array definition
    coreAttribute NewAttribute;
    NewAttribute.iLocation   = iLocation;
    NewAttribute.iComponents = iComponents;
    NewAttribute.iType       = iType;
    NewAttribute.iOffset     = iOffset;

    // add definition to list
    m_aAttribute.push_back(NewAttribute);
}


// ****************************************************************
// activate the vertex structure
void coreVertexBuffer::Activate(const coreByte& iBinding)
{
    SDL_assert(this->GetDataBuffer() && !m_aAttribute.empty());

    if(GLEW_ARB_vertex_attrib_binding)
    {
        // bind the vertex buffer
        glBindVertexBuffer(iBinding, this->GetDataBuffer(), 0, m_iVertexSize);

        FOR_EACH(it, m_aAttribute)
        {
            // enable and specify each defined vertex attribute array
            glEnableVertexAttribArray(it->iLocation);
            glVertexAttribFormat(it->iLocation, it->iComponents, it->iType, false, it->iOffset);
            glVertexAttribBinding(it->iLocation, iBinding);
        }
    }
    else
    {
        // bind the vertex buffer
        this->Bind();

        FOR_EACH(it, m_aAttribute)
        {
            // enable and specify each defined vertex attribute array
            glEnableVertexAttribArray(it->iLocation);
            glVertexAttribPointer(it->iLocation, it->iComponents, it->iType, false, m_iVertexSize, r_cast<const GLvoid*>((long)it->iOffset));
        }
    } 
}