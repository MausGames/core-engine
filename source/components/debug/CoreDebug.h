///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_DEBUG_H_
#define _CORE_GUARD_DEBUG_H_

// TODO 3: ingame editor, with object selection, 3d and 2d, edit position, size and rotation (with axis-control like Blender), edit colors and time-speed
// TODO 3: display direction, orientation and bounding box+radius on objects
// TODO 5: check out AMD GPU PerfAPI
// TODO 3: imgui should not modify debug stats
// TODO 3: mouse-wheel and current focus is not ignored below imgui
// TODO 4: either get rid of FRIEND_CLASS(CoreDebug) or accept it


// ****************************************************************
/* debug definitions */
#define CORE_DEBUG_QUERIES         (4u)        // number of GPU timer-query pairs used for each measure object
#define CORE_DEBUG_SMOOTH_FACTOR   (0.98)      // factor used to smooth-out time values and reduce flickering (higher = smoother, but slower)
#define CORE_DEBUG_OVERALL_NAME    "Overall"   // name for the overall performance output object

#define CORE_DEBUG_STAT_PRIMITIVES (0u)        // submitted primitives
#define CORE_DEBUG_STAT_CLIPPING   (1u)        // primitives output by the clipping stage
#define CORE_DEBUG_STAT_VERTEX     (2u)        // vertex shader invocations
#define CORE_DEBUG_STAT_FRAGMENT   (3u)        // fragment shader invocations
#define CORE_DEBUG_STATS           (4u)        // total number of pipeline statistics


// ****************************************************************
/* main debug component */
class CoreDebug final
{
private:
    /* display structure */
    struct coreDisplay final
    {
        coreUint32   iCurFrame;   // current frame to render
        coreObject2D oOutput;     // object for displaying output

        coreDisplay()noexcept;
    };

    /* measure structure */
    struct coreMeasure final
    {
        coreUint64 iPerfTime;                               // high-precision CPU time value at start
        coreRing<GLuint, CORE_DEBUG_QUERIES> aaiQuery[2];   // asynchronous GPU timer-query objects
        coreDouble dCurrentCPU;                             // current CPU performance value
        coreDouble dCurrentGPU;                             // current GPU performance value
        coreLabel  oOutput;                                 // label for displaying output

        coreMeasure()noexcept;
    };

    /* inspect structure */
    struct coreInspect final
    {
        coreLabel oOutput;   // label for displaying output

        coreInspect()noexcept;
    };

    /* statistic structure */
    struct coreStat final
    {
        GLuint     iQuery;    // asynchronous GPU pipeline-query object
        GLenum     iTarget;   // pipeline-query target (e.g. GL_PRIMITIVES_SUBMITTED_ARB)
        coreUint32 iResult;   // pipeline-query result
        coreUint8  iStatus;   // current processing status (0 = new | 1 = running | 2 = finished)

        coreStat()noexcept;
    };


private:
    coreMap<coreTexture*, coreDisplay*> m_apDisplay;   // display objects to render textures directly on screen
    coreMapStr<coreMeasure*> m_apMeasure;              // measure objects to display CPU and GPU performance values
    coreMapStr<coreInspect*> m_apInspect;              // inspect objects to display current values during run-time
    coreMeasure* m_pOverall;                           // pointer to overall performance output object

    coreObject3D m_DebugCube;                          // debug cube
    coreObject3D m_DebugSphere;                        // debug sphere
    coreObject3D m_DebugVolume;                        // debug collision volume

    coreObject2D m_Background;                         // background object to increase output readability
    coreLabel    m_Loading;                            // resource manager loading indicator

    coreRing<coreStat, CORE_DEBUG_STATS> m_aStat;      // statistic objects to retrieve various pipeline statistics
    coreLabel m_aStatOutput[4];                        // labels for displaying statistic output

    coreBool m_bEnabled;                               // debug-monitor is enabled (debug-build or debug-context)
    coreBool m_bOverlay;                               // debug overlay is visible
    coreBool m_bRendering;                             // debug rendering is visible
    coreBool m_bHolding;                               // holding the current frame


private:
    CoreDebug()noexcept;
    ~CoreDebug();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreDebug)

    /* process debug events */
    coreBool ProcessEvent(const SDL_Event& oEvent);

    /* render textures directly on screen */
    void DisplayTexture(const coreTexturePtr& pTexture, const coreVector2 vSize = coreVector2(0.4f,0.4f));

    /* measure performance between specific points */
    void MeasureStart(const coreHashString& sName);
    void MeasureEnd  (const coreHashString& sName);

    /* inspect and display values during run-time */
    template <typename... A> void InspectValue(const coreHashString& sName, const coreChar* pcFormat, A&&... vArgs);
    inline void InspectValue(const coreHashString& sName, const coreBool    bValue) {this->InspectValue(sName, bValue ? "true" : "false");}
    inline void InspectValue(const coreHashString& sName, const coreInt32   iValue) {this->InspectValue(sName, "%d",                     iValue);}
    inline void InspectValue(const coreHashString& sName, const coreInt64   iValue) {this->InspectValue(sName, "%lld",                   iValue);}
    inline void InspectValue(const coreHashString& sName, const coreUint32  iValue) {this->InspectValue(sName, "%u",                     iValue);}
    inline void InspectValue(const coreHashString& sName, const coreUint64  iValue) {this->InspectValue(sName, "%llu",                   iValue);}
    inline void InspectValue(const coreHashString& sName, const coreFloat   fValue) {this->InspectValue(sName, "%.5f",                   fValue);}
    inline void InspectValue(const coreHashString& sName, const coreVector2 vValue) {this->InspectValue(sName, "%.5f, %.5f",             vValue.x, vValue.y);}
    inline void InspectValue(const coreHashString& sName, const coreVector3 vValue) {this->InspectValue(sName, "%.5f, %.5f, %.5f",       vValue.x, vValue.y, vValue.z);}
    inline void InspectValue(const coreHashString& sName, const coreVector4 vValue) {this->InspectValue(sName, "%.5f, %.5f, %.5f, %.5f", vValue.x, vValue.y, vValue.z, vValue.w);}
    inline void InspectValue(const coreHashString& sName, const void*       pValue) {this->InspectValue(sName, "0x%08X",                 P_TO_UI(pValue));}

    /* render debug objects */
    void RenderCube     (const coreVector3   vPosition, const coreVector3 vSize, const coreVector3 vDirection, const coreVector3 vOrientation);
    void RenderCube     (const coreObject3D* pObject);
    void RenderSphere   (const coreVector3   vPosition, const coreFloat fRadius);
    void RenderSphere   (const coreObject3D* pObject);
    void RenderCollision(const coreObject3D* pObject);

    /* check for debug status */
    static const coreBool& IsEnabled();


private:
    /* retrieve various pipeline statistics */
    void __StatStart();
    void __StatEnd();

    /* update and display debug overlay */
    void __UpdateOutput();

    /* render as wireframe */
    template <typename F> static void __RenderWireframe(F&& nRenderFunc);   // []() -> void
};


// ****************************************************************
/* inspect and display values during run-time */
template <typename... A> void CoreDebug::InspectValue(const coreHashString& sName, const coreChar* pcFormat, A&&... vArgs)
{
    if(!m_bEnabled) return;

    if(!m_apInspect.count(sName))
    {
        // create new inspect object
        coreInspect* pNewInspect = new coreInspect();
        m_apInspect.emplace(sName, pNewInspect);

        // configure output label
        coreLabel& oOutput = pNewInspect->oOutput;
        oOutput.Construct   ("default.ttf", 16u, 0u);
        oOutput.SetCenter   (coreVector2(-0.5f, 0.5f));
        oOutput.SetAlignment(coreVector2( 1.0f,-1.0f));
        oOutput.SetColor3   (COLOR_GREEN);
    }

    // write formatted values to output label
    m_apInspect.at(sName)->oOutput.SetText(PRINT("%s = %s", sName.GetString(), PRINT(pcFormat, std::forward<A>(vArgs)...)));
}


// ****************************************************************
/* render as wireframe */
template <typename F> void CoreDebug::__RenderWireframe(F&& nRenderFunc)
{
    // remember current render state
    GLboolean abStatus[2] = {};
    glGetBooleanv(GL_DEPTH_WRITEMASK, &abStatus[0]);
    glGetBooleanv(GL_CULL_FACE,       &abStatus[1]);

    // enable wireframe mode
    if(CORE_GL_SUPPORT(ANGLE_polygon_mode)) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if(abStatus[0]) glDepthMask(false);
    if(abStatus[1]) glDisable(GL_CULL_FACE);
    {
        // call custom render function
        nRenderFunc();
    }
    if(CORE_GL_SUPPORT(ANGLE_polygon_mode)) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if(abStatus[0]) glDepthMask(true);
    if(abStatus[1]) glEnable(GL_CULL_FACE);
}


#endif /* _CORE_GUARD_DEBUG_H_ */