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
void coreDataBuffer::Create(const GLenum& iTarget, const coreUint& iSize, const void* pData, const GLenum& iUsage)
{
    ASSERT_IF(m_iDataBuffer) this->Delete();

    // save properties
    m_iTarget  = iTarget;
    m_iSize    = iSize;
    m_bDynamic = (iUsage != GL_STATIC_DRAW && iUsage != GL_STATIC_READ && iUsage != GL_STATIC_COPY) ? true : false;

    // generate and bind buffer 
    glGenBuffers(1, &m_iDataBuffer);
    glBindBuffer(iTarget, m_iDataBuffer);
    s_aiBound[iTarget] = m_iDataBuffer; 

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
    
    // reset properties
    m_iDataBuffer = 0;
    m_iTarget     = 0;
    m_iSize       = 0;
    m_bDynamic    = false;
}


// ****************************************************************
// clear content of the data buffer object
void coreDataBuffer::Clear()
{
    ASSERT(m_iDataBuffer && m_bDynamic)

    // clear the whole buffer
    if(GLEW_ARB_clear_buffer_object)
    {
        // TODO: implement this weird function
        //glClearBufferData
    }
}


// ****************************************************************
// invalidate content of the data buffer object
void coreDataBuffer::Invalidate()
{
    ASSERT(m_iDataBuffer && m_bDynamic)

    // invalidate the whole buffer
    if(GLEW_ARB_invalidate_subdata)
    {
        glInvalidateBufferData(m_iDataBuffer);
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
    coreDataBuffer::Create(GL_ARRAY_BUFFER, iNumVertices*iVertexSize, pVertexData, iUsage);

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
void coreVertexBuffer::DefineAttribute(const int& iLocation, const coreByte& iComponents, const GLenum& iType, const coreByte& iOffset)
{
    ASSERT(this->GetDataBuffer())

#if defined(_CORE_DEBUG_)

    // check for duplicate vertex attribute arrays
    FOR_EACH(it, m_aAttribute)
        ASSERT(it->iLocation != iLocation)

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
    ASSERT(this->GetDataBuffer() && !m_aAttribute.empty())

    if(GLEW_ARB_vertex_attrib_binding)
    {
        // bind the vertex buffer
        glBindVertexBuffer(iBinding, this->GetDataBuffer(), 0, m_iVertexSize);

        FOR_EACH(it, m_aAttribute)
        {
            // enable each defined vertex attribute array
            glEnableVertexAttribArray(it->iLocation);
            glVertexAttribBinding(it->iLocation, iBinding);

            // specify the vertex format
            if(it->iType >= GL_BYTE && it->iType <= GL_UNSIGNED_INT)
                glVertexAttribIFormat(it->iLocation, it->iComponents, it->iType,        it->iOffset);
            else glVertexAttribFormat(it->iLocation, it->iComponents, it->iType, false, it->iOffset);
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
            if(it->iType >= GL_BYTE && it->iType <= GL_UNSIGNED_INT)
                glVertexAttribIPointer(it->iLocation, it->iComponents, it->iType,        m_iVertexSize, r_cast<const GLvoid*>((long)it->iOffset));
            else glVertexAttribPointer(it->iLocation, it->iComponents, it->iType, false, m_iVertexSize, r_cast<const GLvoid*>((long)it->iOffset));
        }
    } 
}