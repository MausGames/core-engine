///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


// ****************************************************************
const g_pCanvas   = document.getElementById("canvas");
const g_pLoader   = document.getElementById("loader");
const g_pStatus   = document.getElementById("status");
const g_pProgress = document.getElementById("progress");


// ****************************************************************
g_pCanvas.addEventListener("click", function()
{
    window   .focus();
    g_pCanvas.focus();
});

Module["canvas"] = g_pCanvas;


// ****************************************************************
function PreRun()
{
    FS.mkdir("/user");
    FS.mount(IDBFS, {}, "/user");

    FS.syncfs(true, function(sError)
    {
        if(sError) console.error(sError);
    });
}

Module["preRun"] = (Module["preRun"] || []);
Module["preRun"].push(PreRun);


// ****************************************************************
function PostRun()
{
    FS.syncfs(function(sError)
    {
        if(sError) console.error(sError);
    });
}

Module["postRun"] = (Module["postRun"] || []);
Module["postRun"].push(PostRun);


// ****************************************************************
function SetStatus(sText)
{
    this.sLastText = (this.sLastText || "");

    if(this.sLastText === sText) return;
    this.sLastText = sText;

    const asMatch = sText.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);

    if(asMatch)
    {
        g_pProgress.value = parseInt(asMatch[2]) * 100;
        g_pProgress.max   = parseInt(asMatch[4]) * 100;

        sText = asMatch[1];
    }

    if(sText)
    {
        g_pStatus.innerHTML = sText;
    }
    else
    {
        g_pLoader.style.display = "none";
    }
}

Module["setStatus"] = SetStatus;


// ****************************************************************
function MonitorRunDependencies(iLeft)
{
    this.iOldLeft = (this.iOldLeft || 0);
    this.iTotal   = (this.iTotal   || 0);

    if(this.iOldLeft < iLeft) this.iTotal += iLeft - this.iOldLeft;
    this.iOldLeft = iLeft;

    if(this.iTotal >= 2)
    {
        SetStatus("Preparing... (" + (this.iTotal - iLeft) + "/" + this.iTotal + ")");
    }
}

Module["monitorRunDependencies"] = MonitorRunDependencies;