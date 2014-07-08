//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MODEL_H_
#define _CORE_GUARD_MODEL_H_


// ****************************************************************
// model class
// TODO: support more than one mesh per file
// TODO: add animation
// TODO: support other file formats
// TODO: check for unnecessary calculations to increase loading speed
// TODO: check normal vector calculations (alignment, outside, etc.)
// TODO: (collapse vertices (e.g. same pos) and merge nearly identical vertices in this process (with threshold and std::map)) only adapt normals ?
// TODO: enable shared vertex buffers mixed with unique
// TODO: check out (Multi)Draw(Array|Elements)Indirect
// TODO: reorder indices/vertices in memory to improve post-transform caching (maybe in model-file, nvTriStrip)
// TODO: separate model format from model class, implement old MD3 code for compatibility
class coreModel final : public coreResource
{
private:
    //! MD5-joint structure
    struct md5Joint
    {
        int iParent;                //!< index of the parent joint (-1 = root joint)
        coreVector3 vPosition;      //!< position of the joint
        coreVector4 vOrientation;   //!< orientation of the joint

        explicit md5Joint(const char** ppcData)noexcept;
    };

    //! MD5-vertex structure
    struct md5Vertex
    {
        coreVector2 vTexture;   //!< texture coordinates
        int iWeightStart;       //!< index of the first associated weight
        int iWeightCount;       //!< number of associated weights

        explicit md5Vertex(const char** ppcData)noexcept;
    };

    //! MD5-triangle structure
    struct md5Triangle
    {
        coreWord aiVertex[3];   //!< indexes of the defining vertices

        explicit md5Triangle(const char** ppcData)noexcept;
    };

    //! MD5-weight structure
    struct md5Weight
    {
        int iJoint;              //!< index of the associated joint
        float fBias;             //!< contribution factor
        coreVector3 vPosition;   //!< position of the weight

        explicit md5Weight(const char** ppcData)noexcept;
    };

    //! MD5-mesh structure
    struct md5Mesh
    {
        std::vector<md5Vertex> aVertex;       //!< vertex list
        std::vector<md5Triangle> aTriangle;   //!< triangle list
        std::vector<md5Weight> aWeight;       //!< weight list

        explicit md5Mesh(const char** ppcData)noexcept;
        md5Mesh(md5Mesh&& m)noexcept;
    };

    //! MD5-file structure
    struct md5File
    {
        std::vector<md5Joint> aJoint;   //!< joint list
        std::vector<md5Mesh> aMesh;     //!< mesh list

        explicit md5File(const char** ppcData)noexcept;
        md5File(md5File&& m)noexcept;
    };

    //! vertex structure
    struct coreVertex
    {
        coreVector3 vPosition;   //!< vertex position
        coreVector2 vTexture;    //!< texture coordinate
        coreVector3 vNormal;     //!< normal vector
        coreVector4 vTangent;    //!< additional tangent vector

        constexpr_func coreVertex()noexcept;
    };


private:
    GLuint m_iVertexArray;                              //!< vertex array object

    std::vector<coreVertexBuffer*> m_apiVertexBuffer;   //!< vertex buffers
    coreDataBuffer m_iIndexBuffer;                      //!< index buffer

    coreUint m_iNumVertices;                            //!< number of vertices
    coreUint m_iNumTriangles;                           //!< number of triangles
    coreUint m_iNumIndices;                             //!< number of indices
    float m_fRadius;                                    //!< maximum distance from the model center

    GLenum m_iPrimitiveType;                            //!< primitive type for draw calls (e.g. GL_TRIANGLES)
    GLenum m_iIndexType;                                //!< index type for draw calls (e.g. GL_UNSIGNED_SHORT)

    static coreModel* s_pCurrent;                       //!< currently active model object

    coreSync m_Sync;                                    //!< sync object for asynchronous model loading
    static SDL_SpinLock s_iLock;                        //!< spinlock to prevent asynchronous array buffer access


public:
    coreModel()noexcept;
    ~coreModel();

    //! load and unload model resource data
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! draw the model
    //! @{
    void DrawElements()const;
    void DrawElementsInstanced(const coreUint& iCount)const;
    void DrawArrays()const;
    void DrawArraysInstanced(const coreUint& iCount)const;
    //! @}

    //! enable and disable the model
    //! @{
    void Enable();
    static void Disable(const bool& bFull);
    //! @}

    //! generate custom model resource data
    //! @{
    coreVertexBuffer* CreateVertexBuffer(const coreUint& iNumVertices, const coreByte& iVertexSize, const void* pVertexData, const GLenum& iUsage);
    coreDataBuffer* CreateIndexBuffer(const coreUint& iNumIndices, const coreByte& iIndexSize, const void* pIndexData, const GLenum& iUsage);
    inline coreVertexBuffer* GetVertexBuffer(const coreUint& iID) {return m_apiVertexBuffer[iID];}
    inline coreDataBuffer* GetIndexBuffer()                       {return &m_iIndexBuffer;}
    //! @}

    //! set object properties
    //! @{
    inline void SetRadius(const float& fRadius)                {m_fRadius        = fRadius;}
    inline void SetPrimitiveType(const GLenum& iPrimitiveType) {m_iPrimitiveType = iPrimitiveType;}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint& GetVertexArray()const    {return m_iVertexArray;}
    inline const coreUint& GetNumVertices()const  {return m_iNumVertices;}
    inline const coreUint& GetNumTriangles()const {return m_iNumTriangles;}
    inline const coreUint& GetNumIndices()const   {return m_iNumIndices;}
    inline const float& GetRadius()const          {return m_fRadius;}
    inline const GLenum& GetPrimitiveType()const  {return m_iPrimitiveType;}
    inline const GLenum& GetIndexType()const      {return m_iIndexType;}
    //! @}

    //! get relative path to default resource
    //! @{
    static inline const char* GetDefaultPath() {return "data/models/default.md5mesh";}
    //! @}

    //! get currently active model object
    //! @{
    static inline coreModel* GetCurrent() {return s_pCurrent;}
    //! @}

    //! lock and unlock array buffer access
    //! @{
    static inline void Lock()   {SDL_AtomicLock(&s_iLock);}
    static inline void Unlock() {SDL_AtomicUnlock(&s_iLock);}
    //! @}
};


// ****************************************************************
// constructor
constexpr_func coreModel::coreVertex::coreVertex()noexcept
: vPosition (coreVector3(0.0f,0.0f,0.0f))
, vTexture  (coreVector2(0.0f,0.0f))
, vNormal   (coreVector3(0.0f,0.0f,0.0f))
, vTangent  (coreVector4(0.0f,0.0f,0.0f,0.0f))
{
}


// ****************************************************************
// model resource access type
typedef coreResourcePtr<coreModel> coreModelPtr;


#endif // _CORE_GUARD_MODEL_H_