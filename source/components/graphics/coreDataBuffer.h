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
// TODO: track currently bound data buffer per target (+ assertions)
// TODO: enable read operations (currently only static and write/dynamic)
class coreDataBuffer
{
private:
    GLuint m_iDataBuffer;   //!< data buffer identifier

    GLenum m_iTarget;       //!< buffer target (e.g. GL_ARRAY_BUFFER)
    coreUint m_iSize;       //!< data size in bytes 

    bool m_bDynamic;        //!< storage type


public:
    constexpr_func coreDataBuffer()noexcept;
    ~coreDataBuffer() {this->Delete();}

    //! control the data buffer object
    //! @{
    void Create(const GLenum& iTarget, const coreUint& iSize, const void* pData, const GLenum& iUsage);
    void Delete();
    inline void Bind() {SDL_assert(m_iDataBuffer); glBindBuffer(m_iTarget, m_iDataBuffer);}
    //! @}

    //! modify buffer memory
    //! @{
    coreByte* Map(const coreUint& iOffset, const coreUint& iLength);
    void Unmap(coreByte* pPointer);
    //! @}

    //! access buffer directly
    //! @{
    inline operator const GLuint& ()const noexcept {return m_iDataBuffer;}
    //! @}

    //! get object attributes
    //! @{
    inline const GLuint& GetDataBuffer()const {return m_iDataBuffer;}
    inline const GLenum& GetTarget()const     {return m_iTarget;}
    inline const coreUint& GetSize()const     {return m_iSize;}
    inline const bool& GetDynamic()const      {return m_bDynamic;}
    //! @}


private:
    DISABLE_COPY(coreDataBuffer)
};


// ****************************************************************
// vertex buffer class
// TODO: improve vertex attribute array enable/disable for OGL 2.0 without vertex array objects
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
, m_iTarget     (0)
, m_iSize       (0)
, m_bDynamic    (false)
{
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