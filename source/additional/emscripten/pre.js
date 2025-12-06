///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


// ****************************************************************
const g_pCanvas   = document.getElementById("canvas");
const g_pLoader   = document.getElementById("loader");
const g_pStatus   = document.getElementById("status");
const g_pProgress = document.getElementById("progress");
const g_pMessage  = document.getElementById("message");


// ****************************************************************
g_pCanvas.addEventListener("click", function()
{
    window   .focus();
    g_pCanvas.focus();
});

g_pCanvas.addEventListener("webglcontextcreationerror", function(oEvent)
{
    g_pMessage.innerHTML = "WebGL context creation error: " + oEvent.statusMessage;
});

g_pCanvas.addEventListener("webglcontextlost", function(oEvent)
{
    console.error("WebGL context lost: " + oEvent.statusMessage);
    oEvent.preventDefault();
});

g_pCanvas.addEventListener("webglcontextrestored", function(oEvent)
{
    console.error("WebGL context restored: " + oEvent.statusMessage);
    oEvent.preventDefault();
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
    FS.syncfs(false, function(sError)
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
        const iValue = parseInt(asMatch[2]);
        const iMax   = parseInt(asMatch[4]);

        g_pProgress.style.width = (iValue / iMax) * 100 + "%";

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