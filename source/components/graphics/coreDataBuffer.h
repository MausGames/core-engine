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


// ****************************************************************
// data buffer class
// TODO: enable read operations (currently only static and write/dynamic)
// TODO: implement persistent mapping
class coreDataBuffer
{
private:
    GLuint m_iDataBuffer;                          //!< data buffer identifier
    bool m_bDynamic;                               //!< storage type
                                                         
    GLenum m_iTarget;                              //!< buffer target (e.g. GL_ARRAY_BUFFER)
    coreUint m_iSize;                              //!< data size in bytes 
                                                         
    static coreLookup<GLenum, GLuint> s_aiBound;   //!< data buffer objects currently associated with buffer targets <target, identifier>


public:
    constexpr_func coreDataBuffer()noexcept;
    ~coreDataBuffer() {this->Delete();}

    //! control the data buffer object
    //! @{
    void Create(const GLenum& iTarget, const coreUint& iSize, const void* pData, const GLenum& iUsage);
    void Delete();
    //! @}

    //! bind and unbind the data buffer object
    //! @{
    inline void Bind()const                                               {ASSERT(m_iDataBuffer) coreDataBuffer::Bind(m_iTarget, m_iDataBuffer);}
    static inline void Bind(const GLenum& iTarget, const GLuint& iBuffer) {if(s_aiBound.count(iTarget)) {if(s_aiBound.at(iTarget) == iBuffer) return;} s_aiBound[iTarget] = iBuffer; glBindBuffer(iTarget, iBuffer);}
    static inline void Unbind(const GLenum& iTarget)                      {coreDataBuffer::Bind(iTarget, 0);}
    //! @}

    //! reset content of the data buffer object
    //! @{
    void Clear();
    void Invalidate();
    //! @}

    //! modify buffer memory
    //! @{
    template <typename T> T* Map(const coreUint& iOffset, const coreUint& iLength, const bool& bSync);
    template <typename T> void Unmap(T* pPointer);
    inline const bool& IsDynamic()const {return m_bDynamic;}
    //! @}

    //! access buffer directly
    //! @{
    inline operator const GLuint& ()const noexcept {return m_iDataBuffer;}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint& GetDataBuffer()const {return m_iDataBuffer;}
    inline const GLenum& GetTarget()const     {return m_iTarget;}
    inline const coreUint& GetSize()const     {return m_iSize;}
    //! @}


private:
    DISABLE_COPY(coreDataBuffer)
};


// ****************************************************************
// vertex buffer class
// TODO: improve vertex attribute array enable/disable for OGL (ES) 2.0 without vertex array objects, cache current enabled arrays
class coreVertexBuffer final : public coreDataBuffer
{
private:
    //! vertex attribute array structure
    struct coreAttribute
    {
        int iLocation;          //!< attribute location
        coreByte iComponents;   //!< number of components
        GLenum iType;           //!< component type (e.g. GL_FLOAT)
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
    void Create(const coreUint& iNumVertices, const coreByte& iVertexSize, const void* pVertexData, const GLenum& iUsage);
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
: m_iDataBuffer (0)
, m_bDynamic    (false)
, m_iTarget     (0)
, m_iSize       (0)
{
}


// ****************************************************************
// map buffer memory for writing operations
template <typename T> T* coreDataBuffer::Map(const coreUint& iOffset, const coreUint& iLength, const bool& bSync)
{
    ASSERT(m_iDataBuffer && m_bDynamic && (iOffset+iLength <= m_iSize))

    // bind the data buffer
    this->Bind();
    
    if(GLEW_ARB_map_buffer_range)
    {
        // directly map buffer memory
        return s_cast<T*>(glMapBufferRange(m_iTarget, iOffset, iLength, GL_MAP_WRITE_BIT | (bSync ? GL_MAP_INVALIDATE_RANGE_BIT : GL_MAP_UNSYNCHRONIZED_BIT)));
    }
    else
    {
        // create temporary memory
        T* pPointer = new T[iLength + sizeof(coreUint)*2];

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
constexpr_func coreVertexBuffer::coreAttribute::coreAttribute()noexcept
: iLocation   (0)
, iComponents (0)
, iType       (0)
, iOffset     (0)
{
}        


#endif // _CORE_GUARD_DATABUFFER_H_