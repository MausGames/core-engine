//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
CoreDebug::coreMeasure::coreMeasure()noexcept
: iPerfTime   (0)
, fCurrentCPU (0)
, fCurrentGPU (0)
{
    aaiQuery[0].List()->fill(0);
    aaiQuery[1].List()->fill(0);
}


// ****************************************************************
/* constructor */
CoreDebug::coreInspect::coreInspect()noexcept
{
}


// ****************************************************************
/* constructor */
CoreDebug::CoreDebug()noexcept
: m_pOverall (NULL)
, m_bEnabled (false)
, m_bVisible (false)
{
    if(!Core::Config->GetBool(CORE_CONFIG_SYSTEM_DEBUGMODE) && !DEFINED(_CORE_DEBUG_)) return;

    // enable the debug-monitor
    m_bEnabled = true;

    // create overall performance output object
    this->MeasureStart(CORE_DEBUG_OVERALL_NAME);
    m_pOverall = m_apMeasure.begin()->second;
    m_pOverall->oOutput.SetColor3(COLOR_WHITE);

    // create background object
    m_Background.DefineProgram("default_2d_program");
    m_Background.DefineTexture(0, "default_black.png");
    m_Background.SetPosition  (coreVector2( 0.0f, 0.0f));
    m_Background.SetCenter    (coreVector2(-0.5f, 0.5f));
    m_Background.SetAlignment (coreVector2( 1.0f,-1.0f));
    m_Background.SetColor4    (coreVector4(0.05f,0.05f,0.05f,0.75f));
}


// ****************************************************************
/* destructor */
CoreDebug::~CoreDebug()
{
    // delete all measure and inspect objects
    FOR_EACH(it, m_apMeasure) SAFE_DELETE(it->second)
    FOR_EACH(it, m_apInspect) SAFE_DELETE(it->second)

    // clear memory
    m_apMeasure.clear();
    m_apInspect.clear();
}


// ****************************************************************
/* start measuring performance */
void CoreDebug::MeasureStart(const char* pcName)
{
    if(!m_bEnabled) return;

    if(!m_apMeasure.count(pcName))
    {
        // create new measure object
        coreMeasure* pNewMeasure = new coreMeasure();
        m_apMeasure[pcName] = pNewMeasure;

        if(CORE_GL_SUPPORT(ARB_timer_query))
        {
            // create timer-query objects
            glGenQueries(CORE_DEBUG_QUERIES, pNewMeasure->aaiQuery[0]);
            glGenQueries(CORE_DEBUG_QUERIES, pNewMeasure->aaiQuery[1]);

            // already process later queries to remove invalid values
            for(coreByte i = 1; i < CORE_DEBUG_QUERIES; ++i)
            {
                glQueryCounter(pNewMeasure->aaiQuery[0][i], GL_TIMESTAMP);
                glQueryCounter(pNewMeasure->aaiQuery[1][i], GL_TIMESTAMP);
            }
        }

        // configure output label
        coreLabel& oOutput = pNewMeasure->oOutput;
        oOutput.Construct   ("default.ttf", 16, 64);
        oOutput.SetCenter   (coreVector2(-0.5f, 0.5f));
        oOutput.SetAlignment(coreVector2( 1.0f,-1.0f));
        oOutput.SetColor3   (COLOR_BLUE);
    }

    coreMeasure* pMeasure = m_apMeasure.at(pcName);

    // fetch first CPU time value and start GPU performance measurement
    pMeasure->iPerfTime = SDL_GetPerformanceCounter();
    if(pMeasure->aaiQuery[0][0]) glQueryCounter(pMeasure->aaiQuery[0].GetCur(), GL_TIMESTAMP);
}


// ****************************************************************
/* end measuring performance */
void CoreDebug::MeasureEnd(const char* pcName)
{
    if(!m_bEnabled) return;

    // get requested measure object
    WARN_IF(!m_apMeasure.count(pcName)) return;
    coreMeasure* pMeasure = m_apMeasure.at(pcName);

    // fetch second CPU time value and update CPU performance value
    const float fDifferenceCPU = float(double(SDL_GetPerformanceCounter() - pMeasure->iPerfTime) * Core::System->GetPerfFrequency() * 1.0e03);
    pMeasure->fCurrentCPU = pMeasure->fCurrentCPU * CORE_DEBUG_SMOOTH_FACTOR + fDifferenceCPU * (1.0f-CORE_DEBUG_SMOOTH_FACTOR);

    if(pMeasure->aaiQuery[0][0])
    {
        // end GPU performance measurement
        glQueryCounter(pMeasure->aaiQuery[1].GetCur(), GL_TIMESTAMP);

        // switch to next set of timer-queries (with older already-processed values, asynchronous)
        pMeasure->aaiQuery[0].Next();
        pMeasure->aaiQuery[1].Next();

        // fetch result from both timer-queries
        GLuint64 aiResult[2];
        glGetQueryObjectui64v(pMeasure->aaiQuery[0].GetCur(), GL_QUERY_RESULT, &aiResult[0]);
        glGetQueryObjectui64v(pMeasure->aaiQuery[1].GetCur(), GL_QUERY_RESULT, &aiResult[1]);

        // update GPU performance value
        const float fDifferenceGPU = float(double(aiResult[1] - aiResult[0]) / 1.0e06);
        pMeasure->fCurrentGPU = pMeasure->fCurrentGPU * CORE_DEBUG_SMOOTH_FACTOR + fDifferenceGPU * (1.0f-CORE_DEBUG_SMOOTH_FACTOR);
    }

    if(pMeasure == m_pOverall)
    {
        // add additional performance information (approximated framerate and system utilization)
        const float fMax = MAX(pMeasure->fCurrentCPU, pMeasure->fCurrentGPU);
        pcName = PRINT("%s%s %.1f %.1f%%", pcName, SDL_GL_GetSwapInterval() ? "*" : "", RCP(fMax)*1000.0f, fMax*6.0f);
    }

    // write formatted values to output label
    pMeasure->oOutput.SetText(PRINT("%s (CPU %.2fms, GPU %.2fms)", pcName, pMeasure->fCurrentCPU, pMeasure->fCurrentGPU));
}


// ****************************************************************
/* update and display debug output */
void CoreDebug::__UpdateOutput()
{
    if(!m_bEnabled) return;

    // toggle output visibility
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(F1), CORE_INPUT_PRESS))
        m_bVisible = !m_bVisible;

    // toggle vertical synchronization
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(F2), CORE_INPUT_PRESS))
    {
             if(SDL_GL_GetSwapInterval())   SDL_GL_SetSwapInterval(0);
        else if(SDL_GL_SetSwapInterval(-1)) SDL_GL_SetSwapInterval(1);
    }

    // reset engine
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(F3), CORE_INPUT_PRESS))
    {
        Core::Reset();
        return;
    }

    if(!m_bVisible) return;

    // loop through all objects
    int   iCurLine  = 1;
    float fNewSizeX = m_Background.GetSize().x;

    // move measure objects
    FOR_EACH(it, m_apMeasure)
    {
        it->second->oOutput.SetPosition(coreVector2(0.0f, --iCurLine*0.023f));
        it->second->oOutput.Move();

        fNewSizeX = MAX(fNewSizeX, it->second->oOutput.GetSize().x + 0.005f);
    }

    // move inspect objects
    FOR_EACH(it, m_apInspect)
    {
        it->second->oOutput.SetPosition(coreVector2(0.0f, --iCurLine*0.023f));
        it->second->oOutput.Move();

        fNewSizeX = MAX(fNewSizeX, it->second->oOutput.GetSize().x + 0.005f);
    }

    // move background object (adjust size automatically)
    m_Background.SetSize(coreVector2(fNewSizeX, (1-iCurLine)*0.023f + 0.005f));
    m_Background.Move();

    // hide output on screenshots
    if(!Core::Input->GetKeyboardButton(CORE_INPUT_KEY(PRINTSCREEN), CORE_INPUT_PRESS))
    {
        // render full output
        m_Background.Render();
        FOR_EACH(it, m_apMeasure) it->second->oOutput.Render();
        FOR_EACH(it, m_apInspect) it->second->oOutput.Render();
    }
}