@ECHO OFF

SET _EXECUTABLE_="%~dp0base\glslang.exe"
SET _PARAMETERS_=-t -w

SET _GLOBAL_="%~1\global.glsl"
SET _CUSTOM_="%~1\custom.glsl"
SET _LOG_=shader_log.txt

SET _A_VERSION=(100 110 120 140 150 330 400 410 420 430 440 450)

(@ECHO. ) > %_LOG_%

FOR %%V IN %_A_VERSION% DO (

    (@ECHO [%%V]) >> %_LOG_%

    FOR /R "%~1\" %%G IN (*.vert) DO (

        (@ECHO [%%G]) >> %_LOG_%

        (@ECHO #version                       %%V) >  temp
        (@ECHO #define _CORE_VERTEX_SHADER_     1) >> temp
        (@ECHO #define _CORE_QUALITY_           2) >> temp
        (@ECHO #define CORE_NUM_TEXTURES_2D     4) >> temp
        (@ECHO #define CORE_NUM_TEXTURES_SHADOW 1) >> temp
        (@ECHO #define CORE_NUM_LIGHTS          4) >> temp
        (@ECHO #define CORE_NUM_OUTPUTS         4) >> temp

        COPY /B temp + %_GLOBAL_% + %_CUSTOM_% + "%%G" temp.vert > NUL
        CALL %_EXECUTABLE_% %_PARAMETERS_% temp.vert >> %_LOG_%

    )

    FOR /R "%~1\" %%G IN (*.frag) DO (

        (@ECHO [%%G]) >> %_LOG_%

        (@ECHO #version                       %%V) >  temp
        (@ECHO #define _CORE_FRAGMENT_SHADER_   1) >> temp
        (@ECHO #define _CORE_QUALITY_           2) >> temp
        (@ECHO #define CORE_NUM_TEXTURES_2D     4) >> temp
        (@ECHO #define CORE_NUM_TEXTURES_SHADOW 1) >> temp
        (@ECHO #define CORE_NUM_LIGHTS          4) >> temp
        (@ECHO #define CORE_NUM_OUTPUTS         4) >> temp

        COPY /B temp + %_GLOBAL_% + %_CUSTOM_% + "%%G" temp.frag > NUL
        CALL %_EXECUTABLE_% %_PARAMETERS_% temp.frag >> %_LOG_%

    )

)

DEL temp
DEL temp.vert
DEL temp.frag

START "" %_LOG_%