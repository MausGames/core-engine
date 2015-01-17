//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_DEBUG_H_
#define _CORE_GUARD_DEBUG_H_

// TODO: ingame editor, edit position, size and rotation (with axis-control like Blender), edit colors and time-speed!
// TODO: display direction, orientation and bounding box+radius on objects
// TODO: edit-mode with object selection, 3d and 2d


// ****************************************************************
/* debug definitions */
#define CORE_DEBUG_QUERIES       (4u)        //!< number of GPU timer-query pairs used for each measure object
#define CORE_DEBUG_SMOOTH_FACTOR (0.98f)     //!< factor used to smooth-out time values and reduce flickering (higher = smoother, but slower)
#define CORE_DEBUG_OVERALL_NAME  "Overall"   //!< name for the overall performance output object


// ****************************************************************
/* main debug component */
class CoreDebug final
{
private:
    /*! measure structure */
    struct coreMeasure
    {
        coreUint64 iPerfTime;                                 //!< high precision CPU time value at start
        coreSelect<GLuint, CORE_DEBUG_QUERIES> aaiQuery[2];   //!< asynchronous GPU timer-query objects
        float fCurrentCPU;                                    //!< current CPU performance value
        float fCurrentGPU;                                    //!< current GPU performance value
        coreLabel oOutput;                                    //!< label for displaying output

        coreMeasure()noexcept;
    };

    /*! inspect structure */
    struct coreInspect
    {
        coreLabel oOutput;   //!< label for displaying output

        coreInspect()noexcept;
    };


private:
    coreLookupStr<coreMeasure*> m_apMeasure;   //!< measure objects to display CPU and GPU performance values
    coreLookupStr<coreInspect*> m_apInspect;   //!< inspect objects to display current values during run-time
    coreMeasure* m_pOverall;                   //!< pointer to overall performance output object

    coreObject2D m_Background;                 //!< background object to increase output readability

    bool m_bEnabled;                           //!< debug-monitor is enabled (debug-build or debug-context)
    bool m_bVisible;                           //!< output is visible on screen


private:
    CoreDebug()noexcept;
    ~CoreDebug();
    friend class Core;


public:
    /*! measure performance between specific points */
    //! @{
    void MeasureStart(const char* pcName);
    void MeasureEnd  (const char* pcName);
    //! @}

    /*! inspect and display values during run-time */
    //! @{
    template <typename... A> void InspectValue(const char* pcName, const char* pcFormat, A&&... vArgs);
    inline void InspectValue(const char* pcName, const bool&        bValue) {this->InspectValue(pcName, bValue ? "true" : "false");}
    inline void InspectValue(const char* pcName, const int&         iValue) {this->InspectValue(pcName, "%d",                     iValue);}
    inline void InspectValue(const char* pcName, const float&       fValue) {this->InspectValue(pcName, "%.5f",                   fValue);}
    inline void InspectValue(const char* pcName, const coreVector2& vValue) {this->InspectValue(pcName, "%.5f, %.5f",             vValue.x, vValue.y);}
    inline void InspectValue(const char* pcName, const coreVector3& vValue) {this->InspectValue(pcName, "%.5f, %.5f, %.5f",       vValue.x, vValue.y, vValue.z);}
    inline void InspectValue(const char* pcName, const coreVector4& vValue) {this->InspectValue(pcName, "%.5f, %.5f, %.5f, %.5f", vValue.x, vValue.y, vValue.z, vValue.w);}
    //! @}


private:
    /*! update and display debug output */
    //! @{
    void __UpdateOutput();
    //! @}
};


// ****************************************************************
/* inspect and display values during run-time */
template <typename... A> void CoreDebug::InspectValue(const char* pcName, const char* pcFormat, A&&... vArgs)
{
    if(!m_bEnabled) return;

    if(!m_apInspect.count(pcName))
    {
        // create new inspect object
        coreInspect* pNewInspect = new coreInspect();
        m_apInspect[pcName] = pNewInspect;

        // configure output label
        coreLabel& oOutput = pNewInspect->oOutput;
        oOutput.Construct   ("default.ttf", 16, 64);
        oOutput.SetCenter   (coreVector2(-0.5f, 0.5f));
        oOutput.SetAlignment(coreVector2( 1.0f,-1.0f));
        oOutput.SetColor3   (COLOR_GREEN);
    }

    // write formated values to output label
    m_apInspect.at(pcName)->oOutput.SetText(PRINT("%s (%s)", pcName, PRINT(pcFormat, std::forward<A>(vArgs)...)));
}


#endif /* _CORE_GUARD_DEBUG_H_ */