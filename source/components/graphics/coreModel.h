//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MODEL_H_
#define _CORE_GUARD_MODEL_H_


// ****************************************************************
// model class
// TODO: support more than one mesh per file
// TODO: add animation
// TODO: check for unnecessary calculations to increase loading speed
// TODO: check normal vector calculations (alignment, outside, etc.)
// TODO: make default model invisible
class coreModel final : public coreResource, public coreSync
{
private:
    //! MD5-joint structure
    struct md5Joint
    {
        int iParent;                //!< index of the parent joint (-1 = root joint)
        coreVector3 vPosition;      //!< position of the joint
        coreVector4 vOrientation;   //!< orientation of the joint

        md5Joint(const char** ppcData)noexcept;
    };

    //! MD5-vertex structure
    struct md5Vertex
    {
        coreVector2 vTexture;   //!< texture coordinates
        int iWeightStart;       //!< index of the first associated weight
        int iWeightCount;       //!< number of associated weights

        md5Vertex(const char** ppcData)noexcept;
    };

    //! MD5-triangle structure
    struct md5Triangle
    {
        coreWord aiVertex[3];   //!< indexes of the defining vertices

        md5Triangle(const char** ppcData)noexcept;
    };

    //! MD5-weight structure
    struct md5Weight
    {
        int iJoint;              //!< index of the associated joint
        float fBias;             //!< contribution factor
        coreVector3 vPosition;   //!< position of the weight

        md5Weight(const char** ppcData)noexcept;
    };

    //! MD5-mesh structure
    struct md5Mesh
    {
        std::vector<md5Vertex> aVertex;       //!< vertex list
        std::vector<md5Triangle> aTriangle;   //!< triangle list
        std::vector<md5Weight> aWeight;       //!< weight list

        md5Mesh(const char** ppcData)noexcept;
        md5Mesh(md5Mesh&& m)noexcept;
    };

    //! MD5-file structure
    struct md5File
    {
        std::vector<md5Joint> aJoint;   //!< joint list
        std::vector<md5Mesh> aMesh;     //!< mesh list

        md5File(const char** ppcData)noexcept;
        md5File(md5File&& m)noexcept;
    };

    //! vertex structure
    struct coreVertex
    {
        coreVector3 vPosition;   //!< vertex position
        coreVector2 vTexture;    //!< texture coordinate
        coreVector3 vNormal;     //!< normal vector
        coreVector4 vTangent;    //!< additional tangent vector
    };


private:
    GLuint m_iVertexArray;         //!< vertex array object

    GLuint m_iVertexBuffer;        //!< vertex data buffer
    GLuint m_iIndexBuffer;         //!< index buffer

    coreUint m_iNumVertices;       //!< number of vertices
    coreUint m_iNumTriangles;      //!< number of triangles
    coreUint m_iNumIndices;        //!< number of indices

    float m_fRadius;               //!< maximum distance from the model center

    static SDL_SpinLock s_iLock;   //!< spinlock to prevent asynchronous array buffer access


public:
    coreModel()noexcept;
    explicit coreModel(const char* pcPath)noexcept;
    explicit coreModel(coreFile* pFile)noexcept;
    ~coreModel();

    //! load and unload model resource data
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! draw the model
    //! @{
    void Render();
    //! @}

    //! get object attributes
    //! @{
    inline const GLuint& GetVertexArray()const    {return m_iVertexArray;}
    inline const coreUint& GetNumVertices()const  {return m_iNumVertices;}
    inline const coreUint& GetNumTriangles()const {return m_iNumTriangles;}
    inline const coreUint& GetNumIndices()const   {return m_iNumIndices;}
    inline const float& GetRadius()const          {return m_fRadius;}
    //! @}

    //! get relative path to NULL resource
    //! @{
    static inline const char* GetNullPath() {return "data/models/default.md5mesh";}
    //! @}


private:
    //! define vertex attribute data
    //! @{
    void __BindVertexAttributes();
    //! @}
};


// ****************************************************************
// model resource access type
typedef coreResourcePtr<coreModel> coreModelPtr;


#endif // _CORE_GUARD_MODEL_H_