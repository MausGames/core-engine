///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_RICHTEXT_H_
#define _CORE_GUARD_RICHTEXT_H_

// TODO 3: textures could be made static and shared (clear on reset) (but are bound to style, also static?)
// TODO 3: do not update buffer when just changing max length
// TODO 3: make sure outlines of different characters don't interfere which each other
// TODO 3: maybe handle width-based newline with back-tracking, so not every word/whitespace needs to be forward-checked
// TODO 2: width-based newline check should be able to handle style changes which can affect text width
// TODO 3: do not make newline checks on max-width text
// TODO 3: on rich-text and label, changing rectify inbetween will not update invalidate<>clear handling (only problematic when disabling rectify), which can result in bleeding
// TODO 3: handle extremely big fonts completely filling out a render pass texture


// ****************************************************************
/* menu rich-text definitions */
#define CORE_RICHTEXT_INSTANCE_SIZE_HIGH (32u)   // instancing per-character size (high quality) (position (12), size (4), rotation (4), color (4), texture-parameters (8))
#define CORE_RICHTEXT_INSTANCE_SIZE_LOW  (36u)   // instancing per-character size (low quality)  (position (12), size (8), rotation (4), color (4), texture-parameters (8))
#define CORE_RICHTEXT_INSTANCE_BUFFERS   (3u)    // number of concurrent instance data buffer

#define CORE_RICHTEXT_TEXTURE_SIZE   (coreVector2(512.0f,512.0f))
#define CORE_RICHTEXT_TEXTURE_MARGIN (coreVector2(1.0f,1.0f) / CORE_RICHTEXT_TEXTURE_SIZE)

#define CORE_RICHTEXT_DEFAULT   ("")
#define CORE_RICHTEXT_MAX_ORDER (UINT16_MAX)
#define CORE_RICHTEXT_MAX_WIDTH (1000.0f)

enum coreRichTextRefresh : coreUint8
{
    CORE_RICHTEXT_REFRESH_NOTHING = 0x00u,   // update nothing
    CORE_RICHTEXT_REFRESH_LAYOUT  = 0x01u,   // refresh character layout
    CORE_RICHTEXT_REFRESH_TEXTURE = 0x02u,   // refresh and regenerate texture
    CORE_RICHTEXT_REFRESH_BUFFER  = 0x04u,   // refresh and update instance data buffers
    CORE_RICHTEXT_REFRESH_ALL     = 0x07u    // update everything
};
ENABLE_BITWISE(coreRichTextRefresh)


// ****************************************************************
/* menu rich-text class */
class coreRichText final : public coreObject2D, public coreTranslate, public coreResourceRelation
{
public:
    /* character structure */
    struct coreCharacter final
    {
        coreUint32 iPosition;         // compressed position
        coreUint16 iColorIndex;       // color index
        coreUint16 iTexParamsIndex;   // texture coordinate index
        coreUint16 iOrder;            // order of the character within the text (excluding invisible characters)
    };

    /* animation structure */
    struct coreAnim final
    {
        coreVector2 vPosition;    // interpolated position
        coreVector2 vDirection;   // interpolated direction
        coreVector4 vColor;       // interpolated RGBA color-value
    };

    /* internal types */
    using coreAnimate = coreAnim (*) (const coreCharacter&, void*);


private:
    /* render pass structure */
    struct corePass final
    {
        coreList<coreCharacter> aCharacter;      // arranged characters
        coreTexturePtr          pTexture;        // render pass texture with unique glyphs
        coreList<coreVector4>   avTexParams;     // texture coordinates per glyph (size, offset)
        coreList<coreFloat>     afTexShift;      // additional vertical shift per glyph (for texture generation)
        coreFloat               fHeightFrom;     // top of the current generation line
        coreFloat               fHeightTo;       // bottom of the current generation line
        coreUint16              iNumInstances;   // current instance-capacity of all buffers
        coreUint16              iNumEnabled;     // current number of visible characters (render-count)

        coreRing<GLuint,           CORE_RICHTEXT_INSTANCE_BUFFERS> aiVertexArray;     // vertex array objects
        coreRing<coreVertexBuffer, CORE_RICHTEXT_INSTANCE_BUFFERS> aInstanceBuffer;   // instance data buffers
    };

    /* texture entry structure */
    struct coreEntry final
    {
        coreBool   bGenerated;   // texture generation status
        coreUint8  iPass;        // render pass index
        coreUint16 iIndex;       // texture coordinate index
    };

    /* font style structure */
    struct coreStyle final
    {
        coreFontPtr                    pFont;          // font object
        coreUint16                     iHeight;        // specific height for the font
        coreUint8                      iOutline;       // create very sharp outlined text
        coreList<corePass>             aPass;          // render passes
        coreMap<coreChar32, coreEntry> aEntry;         // texture entries across all render passes
        coreUint8                      iPassIndex;     // current render pass for adding new texture entries
        coreUint16                     iEntryCount;    // number of generated texture entries
        coreProgramPtr                 pProgram;       // shader-program object (regular)
        coreProgramPtr                 pProgramInst;   // shader-program object (instancing)
    };


private:
    coreMapStr<coreStyle>   m_aStyle;    // font style definitions
    coreMapStr<coreVector4> m_avColor;   // color definitions

    coreString m_sText;                  // current text
    coreUint8  m_iRectify;               // align texture with screen pixels (X, Y)
    coreUint16 m_iMaxOrder;              // limit number of characters (excluding invisible characters)
    coreFloat  m_fMaxWidth;              // limit horizontal size and continue into next text line
    coreFloat  m_fLineSkip;              // distance between two text lines

    coreFloat  m_fTopHeight;             // resolution-modified difference between baseline and top of the first text line
    coreUint16 m_iNumLines;              // parsed number of text lines
    coreUint16 m_iNumOrders;             // parsed number or characters (excluding invisible characters)

    void*       m_pAnimateData;          // character animation custom data
    coreAnimate m_nAnimateFunc;          // character animation function

    coreRichTextRefresh m_eRefresh;      // refresh status (dirty flag)


public:
    coreRichText()noexcept;
    ~coreRichText()final;

    DISABLE_COPY(coreRichText)

    /* render and move the rich-text */
    void Render()final;
    void Move  ()final;

    /* handle definitions */
    void AssignStyle (const coreHashString& sName, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline);
    void AssignColor4(const coreHashString& sName, const coreVector4 vColor);
    void AssignColor3(const coreHashString& sName, const coreVector3 vColor);

    /* invoke texture generation */
    void RegenerateTexture();
    void RegenerateTexture(const coreHashString& sStyleName);

    /* override character animation function */
    template <typename F> inline void SetAnimateFunc(void* pAnimateData, F&& nAnimateFunc) {m_pAnimateData = pAnimateData; m_nAnimateFunc = std::forward<F>(nAnimateFunc);}   // [](const coreRichText::coreCharacter& oCharacter, void* pData) -> coreRichText::coreAnim

    /* set object properties */
    coreBool    SetText        (const coreChar*       pcText);
    inline void SetTextLanguage(const coreHashString& sKey, coreAssembleFunc nFunc = NULL) {this->_BindString(&m_sText, sKey, std::move(nFunc));}
    inline void SetRectifyX    (const coreBool        bRectify)                            {if(HAS_FLAG(m_iRectify, 0x01u) != bRectify) {ADD_FLAG(m_eUpdate,  CORE_OBJECT_UPDATE_TRANSFORM) SET_FLAG(m_iRectify, 0x01u, bRectify)}}
    inline void SetRectifyY    (const coreBool        bRectify)                            {if(HAS_FLAG(m_iRectify, 0x02u) != bRectify) {ADD_FLAG(m_eUpdate,  CORE_OBJECT_UPDATE_TRANSFORM) SET_FLAG(m_iRectify, 0x02u, bRectify)}}
    inline void SetRectify     (const coreBool        bRectify)                            {if(HAS_FLAG(m_iRectify, 0x03u) != bRectify) {ADD_FLAG(m_eUpdate,  CORE_OBJECT_UPDATE_TRANSFORM) SET_FLAG(m_iRectify, 0x03u, bRectify)}}
    inline void SetMaxOrder    (const coreUint16&     iMaxOrder)                           {if(m_iMaxOrder != iMaxOrder)                {ADD_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_BUFFER) m_iMaxOrder = iMaxOrder;}}
    inline void SetMaxWidth    (const coreFloat&      fMaxWidth)                           {if(m_fMaxWidth != fMaxWidth)                {ADD_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_LAYOUT) m_fMaxWidth = fMaxWidth;}}
    inline void SetLineSkip    (const coreFloat&      fLineSkip)                           {if(m_fLineSkip != fLineSkip)                {ADD_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_LAYOUT) m_fLineSkip = fLineSkip;}}

    /* get object properties */
    inline const coreChar*   GetText     ()const {return m_sText.c_str();}
    inline       coreUintW   GetTextLen  ()const {return m_sText.length();}
    inline const coreUint8&  GetRectify  ()const {return m_iRectify;}
    inline const coreUint16& GetMaxOrder ()const {return m_iMaxOrder;}
    inline const coreFloat&  GetMaxWidth ()const {return m_fMaxWidth;}
    inline const coreFloat&  GetLineSkip ()const {return m_fLineSkip;}
    inline const coreUint16& GetNumLines ()const {return m_iNumLines;}
    inline const coreUint16& GetNumOrders()const {return m_iNumOrders;}


private:
    /* reset with the resource manager */
    void __Reset(const coreResourceReset eInit)final;

    /* reshape with the resource manager */
    void __Reshape()final;

    /* update object after modification */
    inline void __UpdateTranslate()final {this->RegenerateTexture();}

    /* parse and arrange characters */
    void __ParseText();

    /* generate the texture */
    void __GenerateTexture();

    /* move and adjust the text */
    void __MoveRectified();

    /* handle render passes */
    void __InitPass      (corePass* OUTPUT pPass);
    void __ExitPass      (corePass* OUTPUT pPass);
    void __ReallocatePass(corePass* OUTPUT pPass, const coreUint32 iSize);
};


#endif /* _CORE_GUARD_RICHTEXT_H_ */