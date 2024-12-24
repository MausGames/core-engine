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

    coreObject2D m_Background;                         // background object to increase output readability
    coreLabel    m_Loading;                            // resource manager loading indicator

    coreRing<coreStat, CORE_DEBUG_STATS> m_aStat;      // statistic objects to retrieve various pipeline statistics
    coreLabel m_aStatOutput[3];                        // labels for displaying statistic output

    coreBool m_bEnabled;                               // debug-monitor is enabled (debug-build or debug-context)
    coreBool m_bVisible;                               // output is visible on screen
    coreBool m_bHolding;                               // holding the current frame


private:
    CoreDebug()noexcept;
    ~CoreDebug();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreDebug)

    /* render textures directly on screen */
    void DisplayTexture(const coreTexturePtr& pTexture, const coreVector2 vSize = coreVector2(0.2f,0.2f));

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

    /* check for debug status */
    static const coreBool& IsEnabled();


private:
    /* retrieve various pipeline statistics */
    void __StatStart();
    void __StatEnd();

    /* update and display debug output */
    void __UpdateOutput();
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


#endif /* _CORE_GUARD_DEBUG_H_ */