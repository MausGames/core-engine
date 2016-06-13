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
// TODO: remove per-frame allocation on map/unmap-fallback

// NOTE: superior objects have to handle resource-resets, to refill the buffers


// ****************************************************************
// data buffer definitions
enum coreDataBufferStorage : coreUint16
{
    CORE_DATABUFFER_STORAGE_STATIC     = 0x0001u,   //!< store fast static buffer (STATIC_DRAW)
    CORE_DATABUFFER_STORAGE_DYNAMIC    = 0x0002u,   //!< store writable dynamic buffer (DYNAMIC_DRAW)
    CORE_DATABUFFER_STORAGE_STREAM     = 0x0004u,   //!< store writable temporary buffer (STREAM_DRAW)
    CORE_DATABUFFER_STORAGE_PERSISTENT = 0x0008u,   //!< store persistent mapped buffer when supported (fallback to dynamic)
    CORE_DATABUFFER_STORAGE_FENCED     = 0x0100u    //!< use sync object for reliable asynchronous processing
};
ENABLE_BITWISE(coreDataBufferStorage)

enum coreDataBufferMap : coreUint8
{
    CORE_DATABUFFER_MAP_INVALIDATE_ALL   = GL_MAP_INVALIDATE_BUFFER_BIT,   //!< invalidate complete buffer
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

    GLenum     m_iTarget;                          //!< buffer target (e.g. GL_ARRAY_BUFFER)
    coreUint32 m_iSize;                            //!< data size in bytes

    coreByte*  m_pPersistentBuffer;                //!< pointer to persistent mapped buffer
    coreUint32 m_iMapOffset;                       //!< current mapping offset
    coreUint32 m_iMapLength;                       //!< current mapping length

    coreSync m_Sync;                               //!< sync object for reliable asynchronous processing

    static coreLookup<GLenum, GLuint> s_aiBound;   //!< data buffer objects currently associated with buffer targets <target, identifier>


public:
    constexpr coreDataBuffer()noexcept;
    inline coreDataBuffer(coreDataBuffer&& m)noexcept;
    ~coreDataBuffer();

    //! assignment operations
    //! @{
    coreDataBuffer& operator = (coreDataBuffer o)noexcept;
    //! @}

    //! control the data buffer object
    //! @{
    void Create(const GLenum iTarget, const coreUint32 iSize, const void* pData, const coreDataBufferStorage iStorageType);
    void Delete();
    //! @}

    //! bind and unbind the data buffer object
    //! @{
    inline void Bind()const                                               {ASSERT(m_iDataBuffer) coreDataBuffer::Bind(m_iTarget, m_iDataBuffer);}
    static inline void Bind  (const GLenum iTarget, const GLuint iBuffer) {if(s_aiBound.count(iTarget)) {if(s_aiBound.at(iTarget) == iBuffer) return;} s_aiBound[iTarget] = iBuffer; glBindBuffer(iTarget, iBuffer);}
    static inline void Unbind(const GLenum iTarget, const coreBool bFull) {if(bFull) coreDataBuffer::Bind(iTarget, 0u); else s_aiBound[iTarget] = 0u;}
    //! @}

    //! modify buffer memory
    //! @{
    template <typename T> RETURN_RESTRICT T* Map  (const coreUint32 iOffset, const coreUint32 iLength, const coreDataBufferMap iMapType);
    template <typename T> void               Unmap(T* ptPointer);
    //! @}

    //! reset content of the data buffer object
    //! @{
    void Clear(const coreTextureSpec& oTextureSpec, const void* pData);
    void Invalidate();
    //! @}

    //! check for current buffer status
    //! @{
    inline coreBool IsWritable  ()const {return CONTAINS_FLAG(m_iStorageType, CORE_DATABUFFER_STORAGE_STATIC) ? false :  true;}
    inline coreBool IsPersistent()const {return m_pPersistentBuffer                                           ?  true : false;}
    inline coreBool IsMapped    ()const {return m_iMapLength                                                  ?  true : false;}
    //! @}

    //! access buffer directly
    //! @{
    inline operator GLuint ()const {return m_iDataBuffer;}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint&                GetDataBuffer ()const {return m_iDataBuffer;}
    inline const coreDataBufferStorage& GetStorageType()const {return m_iStorageType;}
    inline const GLenum&                GetTarget     ()const {return m_iTarget;}
    inline const coreUint32&            GetSize       ()const {return m_iSize;}
    //! @}
};


// ****************************************************************
// vertex buffer class
class coreVertexBuffer final : public coreDataBuffer
{
private:
    //! vertex attribute array structure
    struct coreAttribute final
    {
        coreUint8 iLocation;     //!< attribute location
        coreUint8 iComponents;   //!< number of components
        GLenum    iType;         //!< component type (e.g. GL_FLOAT)
        coreBool  bInteger;      //!< pure integer attribute
        coreUint8 iOffset;       //!< offset within the vertex
    };


private:
    coreUint8 m_iVertexSize;                   //!< size of each vertex in bytes
    std::vector<coreAttribute> m_aAttribute;   //!< defined vertex attribute arrays


public:
    coreVertexBuffer()noexcept;
    coreVertexBuffer(coreVertexBuffer&& m)noexcept;
    ~coreVertexBuffer();

    //! assignment operations
    //! @{
    coreVertexBuffer& operator = (coreVertexBuffer o)noexcept;
    //! @}

    //! control the vertex buffer object
    //! @{
    void Create(const coreUint32 iNumVertices, const coreUint8 iVertexSize, const void* pVertexData, const coreDataBufferStorage iStorageType);
    void Delete();
    //! @}

    //! define and activate the vertex structure
    //! @{
    void DefineAttribute(const coreUint8 iLocation, const coreUint8 iComponents, const GLenum iType, const coreBool bInteger, const coreUint8 iOffset);
    void Activate       (const coreUint8 iBinding);
    void ActivateDivided(const coreUint8 iBinding, const coreUint8 iDivisor);
    //! @}

    //! get object properties
    //! @{
    inline const coreUint8& GetVertexSize()const {return m_iVertexSize;}
    //! @}
};


// ****************************************************************
// constructor
constexpr coreDataBuffer::coreDataBuffer()noexcept
: m_iDataBuffer       (0u)
, m_iStorageType      (CORE_DATABUFFER_STORAGE_STATIC)
, m_iTarget           (0u)
, m_iSize             (0u)
, m_pPersistentBuffer (NULL)
, m_iMapOffset        (0u)
, m_iMapLength        (0u)
, m_Sync              ()
{
}

inline coreDataBuffer::coreDataBuffer(coreDataBuffer&& m)noexcept
: m_iDataBuffer       (m.m_iDataBuffer)
, m_iStorageType      (m.m_iStorageType)
, m_iTarget           (m.m_iTarget)
, m_iSize             (m.m_iSize)
, m_pPersistentBuffer (m.m_pPersistentBuffer)
, m_iMapOffset        (m.m_iMapOffset)
, m_iMapLength        (m.m_iMapLength)
, m_Sync              (std::move(m.m_Sync))
{
    m.m_iDataBuffer = 0u;
}


// ****************************************************************
// map buffer memory for writing operations
template <typename T> RETURN_RESTRICT T* coreDataBuffer::Map(const coreUint32 iOffset, const coreUint32 iLength, const coreDataBufferMap iMapType)
{
    ASSERT(m_iDataBuffer && this->IsWritable() && (iOffset+iLength <= m_iSize))

    // save mapping attributes
    m_iMapOffset = iOffset;
    m_iMapLength = iLength;

    // check for sync object status
    if(CONTAINS_FLAG(m_iStorageType, CORE_DATABUFFER_STORAGE_FENCED))
        m_Sync.Check(GL_TIMEOUT_IGNORED, CORE_SYNC_CHECK_NORMAL);

    // return persistent mapped buffer
    if(m_pPersistentBuffer) return r_cast<T*>(m_pPersistentBuffer + iOffset);

    if(CORE_GL_SUPPORT(ARB_map_buffer_range))
    {
        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // map buffer memory directly (new)
            return s_cast<T*>(glMapNamedBufferRange(m_iDataBuffer, iOffset, iLength, GL_MAP_WRITE_BIT | iMapType));
        }
        else if(CORE_GL_SUPPORT(EXT_direct_state_access))
        {
            // map buffer memory directly (old)
            return s_cast<T*>(glMapNamedBufferRangeEXT(m_iDataBuffer, iOffset, iLength, GL_MAP_WRITE_BIT | iMapType));
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
        // create temporary memory
        T* ptPointer = new T[iLength / sizeof(T) + 1u];
        return ptPointer;
    }
}


// ****************************************************************
// unmap buffer memory
template <typename T> void coreDataBuffer::Unmap(T* ptPointer)
{
    ASSERT(ptPointer)

    if(m_pPersistentBuffer)
    {
        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // flush persistent mapped buffer directly (new)
            glFlushMappedNamedBufferRange(m_iDataBuffer, m_iMapOffset, m_iMapLength);
        }
        else if(CORE_GL_SUPPORT(EXT_direct_state_access))
        {
            // flush persistent mapped buffer directly (old)
            glFlushMappedNamedBufferRangeEXT(m_iDataBuffer, m_iMapOffset, m_iMapLength);
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
        if(CORE_GL_SUPPORT(ARB_map_buffer_range))
        {
            if(CORE_GL_SUPPORT(ARB_direct_state_access))
            {
                // unmap buffer memory directly (new)
                glUnmapNamedBuffer(m_iDataBuffer);
            }
            else if(CORE_GL_SUPPORT(EXT_direct_state_access))
            {
                // unmap buffer memory directly (old)
                glUnmapNamedBufferEXT(m_iDataBuffer);
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
            // send new data to the data buffer
            this->Bind();
            glBufferSubData(m_iTarget, m_iMapOffset, m_iMapLength, ptPointer);

            // delete temporary memory
            SAFE_DELETE_ARRAY(ptPointer);
        }
    }

    // create sync object
    if(CONTAINS_FLAG(m_iStorageType, CORE_DATABUFFER_STORAGE_FENCED))
        m_Sync.Create();

    // reset mapping attributes
    m_iMapOffset = 0u;
    m_iMapLength = 0u;
}


#endif // _CORE_GUARD_DATABUFFER_H_