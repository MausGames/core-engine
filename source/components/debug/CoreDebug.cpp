///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
CoreDebug::coreDisplay::coreDisplay()noexcept
: iCurFrame (0u)
, oOutput   ()
{
}


// ****************************************************************
/* constructor */
CoreDebug::coreMeasure::coreMeasure()noexcept
: iPerfTime   (0u)
, aaiQuery    {}
, dCurrentCPU (0.0)
, dCurrentGPU (0.0)
, oOutput     ()
{
}


// ****************************************************************
/* constructor */
CoreDebug::coreInspect::coreInspect()noexcept
: oOutput ()
{
}


// ****************************************************************
/* constructor */
CoreDebug::coreStat::coreStat()noexcept
: iQuery  (0u)
, iTarget (0u)
, iResult (0u)
, iStatus (0u)
{
}


// ****************************************************************
/* constructor */
CoreDebug::CoreDebug()noexcept
: m_apDisplay   {}
, m_apMeasure   {}
, m_apInspect   {}
, m_pOverall    (NULL)
, m_Background  ()
, m_Loading     ()
, m_aStat       {}
, m_aStatOutput ()
, m_bEnabled    (false)
, m_bVisible    (false)
, m_bHolding    (false)
{
    if(!CoreDebug::IsEnabled()) return;

    // enable the debug-monitor
    m_bEnabled = true;

    // create overall performance output object
    this->MeasureStart(CORE_DEBUG_OVERALL_NAME);
    m_pOverall = m_apMeasure.front();
    m_pOverall->oOutput.SetColor3(COLOR_WHITE);

    // create background object
    m_Background.DefineTexture(0u, "default_black.webp");
    m_Background.DefineProgram("default_2d_program");
    m_Background.SetPosition  (coreVector2( 0.0f, 0.0f));
    m_Background.SetCenter    (coreVector2(-0.5f, 0.5f));
    m_Background.SetAlignment (coreVector2( 1.0f,-1.0f));
    m_Background.SetColor4    (coreVector4(0.05f,0.05f,0.05f,0.75f));

    // create loading indicator
    m_Loading.Construct   ("default.ttf", 16u, 0u);
    m_Loading.SetCenter   (coreVector2(-0.5f, 0.5f));
    m_Loading.SetAlignment(coreVector2( 1.0f,-1.0f));
    m_Loading.SetColor3   (COLOR_ORANGE);

    if(CORE_GL_SUPPORT(ARB_pipeline_statistics_query))
    {
        // create pipeline-query objects
        FOR_EACH(it, m_aStat) glGenQueries(1u, &it->iQuery);
        m_aStat[CORE_DEBUG_STAT_PRIMITIVES].iTarget = GL_PRIMITIVES_SUBMITTED;
        m_aStat[CORE_DEBUG_STAT_CLIPPING]  .iTarget = GL_CLIPPING_OUTPUT_PRIMITIVES;
        m_aStat[CORE_DEBUG_STAT_VERTEX]    .iTarget = GL_VERTEX_SHADER_INVOCATIONS;
        m_aStat[CORE_DEBUG_STAT_FRAGMENT]  .iTarget = GL_FRAGMENT_SHADER_INVOCATIONS;

        // configure statistic output labels
        for(coreUintW i = 0u; i < ARRAY_SIZE(m_aStatOutput); ++i)
        {
            m_aStatOutput[i].Construct   ("default.ttf", 16u, 0u);
            m_aStatOutput[i].SetCenter   (coreVector2(-0.5f, 0.5f));
            m_aStatOutput[i].SetAlignment(coreVector2( 1.0f,-1.0f));
            m_aStatOutput[i].SetColor3   (COLOR_PURPLE);
        }
    }

    Core::Log->Info(CORE_LOG_BOLD("Debug Component enabled"));
}


// ****************************************************************
/* destructor */
CoreDebug::~CoreDebug()
{
    if(!m_bEnabled) return;

    if(CORE_GL_SUPPORT(ARB_timer_query))
    {
        // delete timer-query objects
        FOR_EACH(it, m_apMeasure)
        {
            glDeleteQueries(CORE_DEBUG_QUERIES, (*it)->aaiQuery[0].data());
            glDeleteQueries(CORE_DEBUG_QUERIES, (*it)->aaiQuery[1].data());
        }
    }

    if(CORE_GL_SUPPORT(ARB_pipeline_statistics_query))
    {
        // delete pipeline-query objects (also finish the last query, or it may crash)
        this->__StatEnd();
        FOR_EACH(it, m_aStat) glDeleteQueries(1u, &it->iQuery);
    }

    // delete all display, measure and inspect objects
    FOR_EACH(it, m_apDisplay) SAFE_DELETE(*it)
    FOR_EACH(it, m_apMeasure) SAFE_DELETE(*it)
    FOR_EACH(it, m_apInspect) SAFE_DELETE(*it)

    // clear memory
    m_apDisplay.clear();
    m_apMeasure.clear();
    m_apInspect.clear();

    Core::Log->Info(CORE_LOG_BOLD("Debug Component disabled"));
}


// ****************************************************************
/* process debug events */
coreBool CoreDebug::ProcessEvent(const SDL_Event& oEvent)
{
    return true;
}


// ****************************************************************
/* render texture directly on screen */
void CoreDebug::DisplayTexture(const coreTexturePtr& pTexture, const coreVector2 vSize)
{
    if(!m_bEnabled) return;

    if(!m_apDisplay.count(&(*pTexture)))
    {
        // create new display object
        coreDisplay* pNewDisplay = new coreDisplay();
        m_apDisplay.emplace(&(*pTexture), pNewDisplay);

        // configure output object
        coreObject2D& oOutput = pNewDisplay->oOutput;
        oOutput.DefineProgram("default_2d_program");
        oOutput.SetSize      (vSize);
        oOutput.SetCenter    (coreVector2(-0.5f,-0.5f));
        oOutput.SetAlignment (coreVector2( 1.0f, 1.0f));
    }

    // save current frame (only render once, to prevent invalid access)
    m_apDisplay.at(&(*pTexture))->iCurFrame = Core::System->GetCurFrame();
}


// ****************************************************************
/* start measuring performance */
void CoreDebug::MeasureStart(const coreHashString& sName)
{
    if(!m_bEnabled) return;

    if(!m_apMeasure.count(sName))
    {
        // create new measure object
        coreMeasure* pNewMeasure = new coreMeasure();
        m_apMeasure.emplace(sName, pNewMeasure);

        if(CORE_GL_SUPPORT(ARB_timer_query))
        {
            // create timer-query objects
            glGenQueries(CORE_DEBUG_QUERIES, pNewMeasure->aaiQuery[0].data());
            glGenQueries(CORE_DEBUG_QUERIES, pNewMeasure->aaiQuery[1].data());

            // already process later queries to remove invalid values
            for(coreUintW i = 1u; i < CORE_DEBUG_QUERIES; ++i)
            {
                glQueryCounter(pNewMeasure->aaiQuery[0][i], GL_TIMESTAMP);
                glQueryCounter(pNewMeasure->aaiQuery[1][i], GL_TIMESTAMP);
            }
        }

        // configure output label
        coreLabel& oOutput = pNewMeasure->oOutput;
        oOutput.Construct   ("default.ttf", 16u, 0u);
        oOutput.SetCenter   (coreVector2(-0.5f, 0.5f));
        oOutput.SetAlignment(coreVector2( 1.0f,-1.0f));
        oOutput.SetColor3   (COLOR_BLUE);
    }

    coreMeasure* pMeasure = m_apMeasure.at(sName);

    // fetch first CPU time value and start GPU performance measurement
    pMeasure->iPerfTime = SDL_GetPerformanceCounter();
    if(pMeasure->aaiQuery[0][0]) glQueryCounter(pMeasure->aaiQuery[0].current(), GL_TIMESTAMP);

    // start pipeline statistics
    if(pMeasure == m_pOverall) this->__StatStart();
}


// ****************************************************************
/* end measuring performance */
void CoreDebug::MeasureEnd(const coreHashString& sName)
{
    if(!m_bEnabled) return;

    // get requested measure object
    WARN_IF(!m_apMeasure.count(sName)) return;
    coreMeasure* pMeasure = m_apMeasure.at(sName);

    if(pMeasure->aaiQuery[0][0])
    {
        // end GPU performance measurement
        glQueryCounter(pMeasure->aaiQuery[1].current(), GL_TIMESTAMP);

        // switch to next set of timer-queries (with older already-processed values, asynchronous)
        pMeasure->aaiQuery[0].next();
        pMeasure->aaiQuery[1].next();

        // fetch result from both timer-queries
        GLuint64 aiResult[2];
        glGetQueryObjectui64v(pMeasure->aaiQuery[0].current(), GL_QUERY_RESULT, &aiResult[0]);
        glGetQueryObjectui64v(pMeasure->aaiQuery[1].current(), GL_QUERY_RESULT, &aiResult[1]);

        // update GPU performance value
        const coreDouble dDifferenceGPU = coreDouble(aiResult[1] - aiResult[0]) / 1.0e06;
        pMeasure->dCurrentGPU = pMeasure->dCurrentGPU * CORE_DEBUG_SMOOTH_FACTOR + dDifferenceGPU * (1.0-CORE_DEBUG_SMOOTH_FACTOR);
    }

    const coreChar* pcName = sName.GetString();
    if(pMeasure == m_pOverall)
    {
        // end pipeline statistics
        this->__StatEnd();

        // add additional performance information (frame rate and process memory)
        const coreFloat fTime = Core::System->GetTime();
        if(fTime) pcName = PRINT("%s %.1f FPS%s %.2f MB / %.2f MB", pcName, RCP(fTime), SDL_GL_GetSwapIntervalInline() ? "*" : "", coreDouble(coreData::ProcessMemory()) / (1024.0*1024.0), coreDouble(Core::Graphics->ProcessGpuMemory()) / (1024.0*1024.0));
    }

    // fetch second CPU time value and update CPU performance value
    const coreDouble dDifferenceCPU = coreDouble(SDL_GetPerformanceCounter() - pMeasure->iPerfTime) * Core::System->GetPerfFrequency() * 1.0e03;
    pMeasure->dCurrentCPU = pMeasure->dCurrentCPU * CORE_DEBUG_SMOOTH_FACTOR + dDifferenceCPU * (1.0-CORE_DEBUG_SMOOTH_FACTOR);

    // write formatted values to output label
    pMeasure->oOutput.SetText(PRINT("%s (CPU %.2fms / GPU %.2fms)", pcName, pMeasure->dCurrentCPU, pMeasure->dCurrentGPU));
}


// ****************************************************************
/* check for debug status */
const coreBool& CoreDebug::IsEnabled()
{
    ASSERT(STATIC_ISVALID(Core::Config))

    // only check once
    static const coreBool s_bEnabled = Core::Config->GetBool(CORE_CONFIG_BASE_DEBUGMODE) || DEFINED(_CORE_DEBUG_);
    return s_bEnabled;
}


// ****************************************************************
/* start pipeline statistics */
void CoreDebug::__StatStart()
{
    if(CORE_GL_SUPPORT(ARB_pipeline_statistics_query))
    {
        // limit the processing frequency (because of high performance impact)
        if(SDL_GL_GetSwapIntervalInline() || (F_TO_UI(Core::System->GetTotalTimeFloat(100.0) * 4.0f) % m_aStat.size() != m_aStat.index()))
        {
            // switch to next statistic object (only process one at a time, or it may crash)
            m_aStat.next();

            if(m_aStat.current().iStatus == 2u)
            {
                // fetch result from the pipeline-query
                glGetQueryObjectuiv(m_aStat.current().iQuery, GL_QUERY_RESULT, &m_aStat.current().iResult);

                // write primitive statistics
                if(((m_aStat.index() == CORE_DEBUG_STAT_PRIMITIVES) ||
                    (m_aStat.index() == CORE_DEBUG_STAT_CLIPPING))  &&
                    (m_aStat[CORE_DEBUG_STAT_PRIMITIVES].iResult))
                {
                    m_aStatOutput[0].SetText(PRINT("Primitives: %'u (%.1f%% clipped)", m_aStat[CORE_DEBUG_STAT_PRIMITIVES].iResult, (1.0f - I_TO_F(m_aStat[CORE_DEBUG_STAT_CLIPPING].iResult) * RCP(I_TO_F(m_aStat[CORE_DEBUG_STAT_PRIMITIVES].iResult))) * 100.0f));
                }

                // write vertex statistics
                if(((m_aStat.index() == CORE_DEBUG_STAT_VERTEX)      ||
                    (m_aStat.index() == CORE_DEBUG_STAT_PRIMITIVES)) &&
                    (m_aStat[CORE_DEBUG_STAT_PRIMITIVES].iResult))
                {
                    m_aStatOutput[1].SetText(PRINT("Vertex Invocations: %'u (%.2f per primitive)", m_aStat[CORE_DEBUG_STAT_VERTEX].iResult, I_TO_F(m_aStat[CORE_DEBUG_STAT_VERTEX].iResult) * RCP(I_TO_F(m_aStat[CORE_DEBUG_STAT_PRIMITIVES].iResult))));
                }

                // write fragment statistics
                if(m_aStat.index() == CORE_DEBUG_STAT_FRAGMENT)
                {
                    m_aStatOutput[2].SetText(PRINT("Fragment Invocations: %'u (%.2f per pixel)", m_aStat[CORE_DEBUG_STAT_FRAGMENT].iResult, I_TO_F(m_aStat[CORE_DEBUG_STAT_FRAGMENT].iResult) * RCP(Core::System->GetResolution().x * Core::System->GetResolution().y)));
                }
            }

            // start GPU pipeline-query processing
            glBeginQuery(m_aStat.current().iTarget, m_aStat.current().iQuery);
            m_aStat.current().iStatus = 1u;
        }
    }
}


// ****************************************************************
/* end pipeline statistics */
void CoreDebug::__StatEnd()
{
    if(m_aStat.current().iStatus == 1u)
    {
        // end GPU pipeline-query processing
        glEndQuery(m_aStat.current().iTarget);
        m_aStat.current().iStatus = 2u;
    }
}


// ****************************************************************
/* update and display debug output */
void CoreDebug::__UpdateOutput()
{
    if(!m_bEnabled) return;

    // toggle output visibility
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(F1), CORE_INPUT_PRESS) || Core::Input->GetJoystickButton(CORE_INPUT_JOYSTICK_ANY, SDL_GAMEPAD_BUTTON_LEFT_STICK, CORE_INPUT_PRESS))
        m_bVisible = !m_bVisible;

    // toggle vertical synchronization
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(F2), CORE_INPUT_PRESS))
    {
        SDL_GL_SetSwapInterval(SDL_GL_GetSwapIntervalInline() ? 0 : 1);
    }

    // hold and skip frame
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(F5), CORE_INPUT_PRESS))
        m_bHolding = !m_bHolding;
    if(m_bHolding && !Core::Input->GetKeyboardButton(CORE_INPUT_KEY(F6), CORE_INPUT_PRESS))
        Core::System->SkipFrame();

    // reset language
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(F9), CORE_INPUT_PRESS))
        Core::Language->Load(Core::Language->GetPath());

    // reset resources
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(F10), CORE_INPUT_PRESS))
    {
        Core::Manager::Resource->Reset();
        return;
    }

    // reset engine
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(F11), CORE_INPUT_PRESS))
    {
        Core::Reset();
        return;
    }

    if(!m_bVisible) return;

    // loop through all objects
    coreInt8  iCurLine  = 1;
    coreFloat fNewSizeX = m_Background.GetSize().x;

    // move measure objects
    FOR_EACH(it, m_apMeasure)
    {
        coreMeasure* pMeasure = (*it);

        pMeasure->oOutput.SetPosition(coreVector2(0.0f, I_TO_F(--iCurLine)*0.023f));
        pMeasure->oOutput.Move();

        fNewSizeX = MAX(fNewSizeX, pMeasure->oOutput.GetSize().x + 0.005f);
    }

    // move inspect objects
    FOR_EACH(it, m_apInspect)
    {
        coreInspect* pInspect = (*it);

        pInspect->oOutput.SetPosition(coreVector2(0.0f, I_TO_F(--iCurLine)*0.023f));
        pInspect->oOutput.Move();

        fNewSizeX = MAX(fNewSizeX, pInspect->oOutput.GetSize().x + 0.005f);
    }

    // move statistic output labels
    if(CORE_GL_SUPPORT(ARB_pipeline_statistics_query))
    {
        for(coreUintW i = 0u; i < ARRAY_SIZE(m_aStatOutput); ++i)
        {
            m_aStatOutput[i].SetPosition(coreVector2(0.0f, I_TO_F(--iCurLine)*0.023f));
            m_aStatOutput[i].Move();
        }
    }

    // move loading indicator
    const coreUintW iLoadingNum = Core::Manager::Resource->IsLoadingNum();
    if(iLoadingNum)
    {
        m_Loading.SetPosition(coreVector2(0.0f, I_TO_F(--iCurLine)*0.023f));
        m_Loading.SetText(PRINT("Loading (%zu)", iLoadingNum));
        m_Loading.Move();
    }

    // move display objects
    coreFloat fOffsetX = 0.0f;
    FOR_EACH_DYN(it, m_apDisplay)
    {
        coreDisplay* pDisplay = (*it);

        // remove old entries
        if(pDisplay->iCurFrame != Core::System->GetCurFrame())
        {
            SAFE_DELETE(*it)
            DYN_REMOVE(it, m_apDisplay)
        }
        else
        {
            pDisplay->oOutput.SetPosition(coreVector2(fOffsetX, 0.0f));
            pDisplay->oOutput.Move();

            fOffsetX += pDisplay->oOutput.GetSize().x;
            DYN_KEEP(it, m_apDisplay)
        }
    }

    // move background object (adjust size automatically)
    m_Background.SetSize(coreVector2(fNewSizeX, I_TO_F(1-iCurLine)*0.023f + 0.005f));
    m_Background.Move();

    // hide output on screenshots
    if(!Core::Input->GetKeyboardButton(CORE_INPUT_KEY(PRINTSCREEN), CORE_INPUT_PRESS))
    {
        glDisable(GL_DEPTH_TEST);
        {
            // render image output
            FOR_EACH(it, m_apDisplay)
            {
                (*m_apDisplay.get_key(it))->Enable(0u);
                (*it)->oOutput.Render();
            }

            // render text output
            m_Background.Render();
            FOR_EACH(it, m_apMeasure) (*it)->oOutput.Render();
            FOR_EACH(it, m_apInspect) (*it)->oOutput.Render();
            if(iLoadingNum) m_Loading.Render();

            // render statistic text output
            if(CORE_GL_SUPPORT(ARB_pipeline_statistics_query))
            {
                for(coreUintW i = 0u; i < ARRAY_SIZE(m_aStatOutput); ++i)
                    m_aStatOutput[i].Render();
            }
        }
        glEnable(GL_DEPTH_TEST);
    }
}