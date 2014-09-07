//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_DATABUFFER_H_
#define _CORE_GUARD_DATABUFFER_H_

// TODO: enable read and copy operations (currently only static and write/dynamic)
// TODO: improve vertex attribute array enable/disable for OGL (ES) 2.0 without vertex array objects, cache current enabled arrays

// NOTE: superior objects have to handle resource-resets, to refill the buffers
// NOTE: notated as integer
// NOTE: uniform buffers should change only once per frame, otherwise it either stalls or is unsynchronized


// ****************************************************************
// data buffer definitions
enum coreDataBufferStorage : coreByte
{
    CORE_DATABUFFER_STORAGE_STATIC     = 0,   //!< store fast static buffer (STATIC_DRAW)
    CORE_DATABUFFER_STORAGE_DYNAMIC    = 1,   //!< store writable dynamic buffer (DYNAMIC_DRAW)
    CORE_DATABUFFER_STORAGE_STREAM     = 2,   //!< store writable temporary buffer (STREAM_DRAW)
    CORE_DATABUFFER_STORAGE_PERSISTENT = 3    //!< store persistent mapped buffer when supported (fallback to dynamic)
};

enum coreDataBufferMap : coreByte
{
    CORE_DATABUFFER_MAP_INVALIDATE_ALL   = GL_MAP_INVALIDATE_BUFFER_BIT,   //!< invalidate complete buffer for max performance
    CORE_DATABUFFER_MAP_INVALIDATE_RANGE = GL_MAP_INVALIDATE_RANGE_BIT,    //!< invalidate only required range
    CORE_DATABUFFER_MAP_UNSYNCHRONIZED   = GL_MAP_UNSYNCHRONIZED_BIT       //!< map and unmap unsynchronized
};


// ****************************************************************
// data buffer class
class coreDataBuffer
{
private:
    GLuint m_iDataBuffer;                          //!< data buffer identifier
    coreDataBufferStorage m_iStorageType;          //!< storage type
                                                         
    GLenum   m_iTarget;                            //!< buffer target (e.g. GL_ARRAY_BUFFER)
    coreUint m_iSize;                              //!< data size in bytes 

    coreByte* m_pPersistentBuffer;                 //!< pointer to persistent mapped buffer
                                                         
    static coreLookup<GLenum, GLuint> s_aiBound;   //!< data buffer objects currently associated with buffer targets <target, identifier>


public:
    constexpr_func coreDataBuffer()noexcept;
    ~coreDataBuffer() {this->Delete();}

    //! control the data buffer object
    //! @{
    void Create(const GLenum& iTarget, const coreUint& iSize, const void* pData, const coreDataBufferStorage& iStorageType);
    void Delete();
    //! @}

    //! bind and unbind the data buffer object
    //! @{
    inline void Bind()const                                                 {ASSERT(m_iDataBuffer) coreDataBuffer::Bind(m_iTarget, m_iDataBuffer);}
    static inline void Bind  (const GLenum& iTarget, const GLuint& iBuffer) {if(s_aiBound.count(iTarget)) {if(s_aiBound.at(iTarget) == iBuffer) return;} s_aiBound[iTarget] = iBuffer; glBindBuffer(iTarget, iBuffer);}
    static inline void Unbind(const GLenum& iTarget, const bool& bFull)     {if(bFull) coreDataBuffer::Bind(iTarget, 0); else s_aiBound[iTarget] = 0;}
    //! @}

    //! modify buffer memory
    //! @{
    template <typename T> T*   Map  (const coreUint& iOffset, const coreUint& iLength, const coreDataBufferMap& iMapType);
    template <typename T> void Unmap(T* pPointer);
    inline bool IsWritable()const {return (m_iStorageType != CORE_DATABUFFER_STORAGE_STATIC) ? true : false;}
    //! @}

    //! reset content of the data buffer object
    //! @{
    void Clear(const GLenum& iInternal, const GLenum& iFormat, const GLenum& iType, const void* pData);
    void Invalidate();
    //! @}

    //! access buffer directly
    //! @{
    inline operator const GLuint& ()const {return m_iDataBuffer;}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint&                GetDataBuffer ()const {return m_iDataBuffer;}
    inline const coreDataBufferStorage& GetStorageType()const {return m_iStorageType;}
    inline const GLenum&                GetTarget     ()const {return m_iTarget;}
    inline const coreUint&              GetSize       ()const {return m_iSize;}
    //! @}


private:
    DISABLE_COPY(coreDataBuffer)
};


// ****************************************************************
// vertex buffer class
class coreVertexBuffer final : public coreDataBuffer
{
private:
    //! vertex attribute array structure
    struct coreAttribute
    {
        int      iLocation;     //!< attribute location
        coreByte iComponents;   //!< number of components
        GLenum   iType;         //!< component type (e.g. GL_FLOAT)
        coreByte iOffset;       //!< offset within the vertex

        constexpr_func coreAttribute()noexcept;
    };


private:
    coreByte m_iVertexSize;                    //!< size of each vertex in bytes
    std::vector<coreAttribute> m_aAttribute;   //!< defined vertex attribute arrays


public:
    coreVertexBuffer()noexcept;
    ~coreVertexBuffer() {this->Delete();}

    //! control the vertex buffer object
    //! @{
    void Create(const coreUint& iNumVertices, const coreByte& iVertexSize, const void* pVertexData, const coreDataBufferStorage& iStorageType);
    void Delete();
    //! @}

    //! define and activate the vertex structure
    //! @{
    void DefineAttribute(const int& iLocation, const coreByte& iComponents, const GLenum& iType, const coreByte& iOffset);
    void Activate(const coreByte& iBinding);
    //! @}
};


// ****************************************************************
// constructor
constexpr_func coreDataBuffer::coreDataBuffer()noexcept
: m_iDataBuffer       (0)
, m_iStorageType      (CORE_DATABUFFER_STORAGE_STATIC)
, m_iTarget           (0)
, m_iSize             (0)
, m_pPersistentBuffer (NULL)
{
}


// ****************************************************************
// map buffer memory for writing operations
template <typename T> T* coreDataBuffer::Map(const coreUint& iOffset, const coreUint& iLength, const coreDataBufferMap& iMapType)
{
    ASSERT(m_iDataBuffer && this->IsWritable() && (iOffset+iLength <= m_iSize))

    // return persistent mapped buffer
    if(m_pPersistentBuffer)
        return r_cast<T*>(m_pPersistentBuffer + iOffset);

    if(CORE_GL_SUPPORT(ARB_map_buffer_range))
    {
        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // map buffer memory directly
            return s_cast<T*>(glMapNamedBufferRange(m_iDataBuffer, iOffset, iLength, GL_MAP_WRITE_BIT | iMapType));
        }
        else
        {
            // bind and map buffer memory
            this->Bind();
            return s_cast<T*>(glMapBufferRange(m_iTarget, iOffset, iLength, GL_MAP_WRITE_BIT | iMapType));
        }
    }
    else
    {
        // create temporary memory (attribute memory may not be tight)
        T* pPointer = new T[iLength / sizeof(T) + sizeof(coreUint)*2];

        // add mapping attributes
        std::memcpy(pPointer,                    &iOffset, sizeof(coreUint));
        std::memcpy(pPointer + sizeof(coreUint), &iLength, sizeof(coreUint));

        return pPointer + sizeof(coreUint)*2;
    }
}


// ****************************************************************
// unmap buffer memory
template <typename T> void coreDataBuffer::Unmap(T* pPointer)
{
    ASSERT(pPointer)

    // keep persistent mapped buffer
    if(m_pPersistentBuffer) return;

    if(CORE_GL_SUPPORT(ARB_map_buffer_range))
    {
        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // unmap buffer memory directly
            glUnmapNamedBuffer(m_iDataBuffer);
        }
        else
        {
            // bind and unmap buffer memory
            this->Bind();
            glUnmapBuffer(m_iTarget);
        }
    }
    else
    {
        // extract mapping attributes
        coreUint iOffset; std::memcpy(&iOffset, pPointer - sizeof(coreUint)*2, sizeof(coreUint));
        coreUint iLength; std::memcpy(&iLength, pPointer - sizeof(coreUint)*1, sizeof(coreUint));

        // send new data to the data buffer
        this->Bind();
        glBufferSubData(m_iTarget, iOffset, iLength, pPointer);

        // delete temporary memory
        pPointer -= sizeof(coreUint)*2;
        SAFE_DELETE_ARRAY(pPointer);
    }
}


// ****************************************************************
// constructor
constexpr_func coreVertexBuffer::coreAttribute::coreAttribute()noexcept
: iLocation   (0)
, iComponents (0)
, iType       (0)
, iOffset     (0)
{
}        


#endif // _CORE_GUARD_DATABUFFER_H_