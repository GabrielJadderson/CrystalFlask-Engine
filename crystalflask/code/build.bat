@echo off

set GladCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi /fp:fast /fp:except-
set ImguiCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi /fp:fast /fp:except-

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi /fp:fast /fp:except- -W4 -wd4530 -wd4211 -wd4201 -wd4100 -wd4996 -wd4127 -wd4505 -wd4189 -wd4456 -DCRYSTALFLASK_INTERNAL=1 -DCRYSTALFLASK_SLOW=1 -DCRYSTALFLASK_WIN32=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib shell32.lib  winmm.lib opengl32.lib Comdlg32.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

REM 64-bit build
del *.pdb > NUL 2> NUL
REM Optimization switches /O2 /Oi /fp:fast


set stbIncludes=-I"..\crystalflask\code\ThirdParty\stb"
set glmIncludes=-I"..\crystalflask\code\ThirdParty\glm"
set ImguiIncludes=-I"..\crystalflask\code\ThirdParty\imgui"
set AssimpIncludes=-I"..\crystalflask\code\ThirdParty\assimp\include"

set GLIncludes=-I"..\crystalflask\code\ThirdParty\GL"
set GLSrc="..\crystalflask\code\ThirdParty\GL\gl3w.c"

REM Building gl3w
IF NOT EXIST gl3w.obj (
echo WAITING FOR PDB > gl3wlock.tmp
CALL :__START_TIME_MEASURE
echo Building gl3w
cl -c %GLIncludes% %ImguiCompilerFlags% %GLSrc%
CALL :__STOP_TIME_MEASURE
del gl3wlock.tmp
) ELSE (
 REM found gl3w
)

set AssimpLibs="..\crystalflask\code\ThirdParty\assimp\lib\assimp-vc142-mt.lib" "..\crystalflask\code\ThirdParty\assimp\lib\IrrXML.lib" "..\crystalflask\code\ThirdParty\assimp\lib\zlibstatic.lib"

REM Building ASSIMP
IF NOT EXIST assimp-vc142-mt.dll (
echo WAITING FOR PDB > assimplock.tmp
copy "..\crystalflask\code\ThirdParty\assimp\bin\assimp-vc142-mt.dll" . >NUL
echo Copied assimp-vc142-mt.dll
del assimplock.tmp
) ELSE (
 REM found imgui
)


set ImguiSrc="..\crystalflask\code\ThirdParty\imgui\imgui.cpp" "..\crystalflask\code\ThirdParty\imgui\imgui_demo.cpp" "..\crystalflask\code\ThirdParty\imgui\imgui_draw.cpp" "..\crystalflask\code\ThirdParty\imgui\imgui_widgets.cpp"

set ImguiObjs="imgui.obj" "imgui_demo.obj" "imgui_draw.obj" "imgui_widgets.obj"

REM Building Resource file
IF NOT EXIST crystalflask.res (
echo WAITING FOR PDB > resourcelock.tmp
copy "..\crystalflask\code\crystalflask.rc" . >NUL
rc -nologo crystalflask.rc >NUL
echo Copied crystalflask.rc
echo Generated crystalflask.res
del resourcelock.tmp
) ELSE (
 REM found resource
)



REM Building IMGUI
IF NOT EXIST imgui.obj (
echo WAITING FOR PDB > imguilock.tmp
CALL :__START_TIME_MEASURE
echo Building ImGui
cl -c %ImguiIncludes% %ImguiCompilerFlags% %ImguiSrc%
CALL :__STOP_TIME_MEASURE
del imguilock.tmp
) ELSE (
 REM found imgui
)


REM Building crystalflask
echo WAITING FOR PDB > lock.tmp
CALL :__START_TIME_MEASURE
REM echo Building crystalflask
cl %GLIncludes% %glmIncludes% %CommonCompilerFlags% ..\crystalflask\code\crystalflask.cpp -Fmcrystalflask.map -LD /link -incremental:no -opt:ref -PDB:crystalflask_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender
CALL :__STOP_TIME_MEASURE
del lock.tmp
echo.



set Win32Includes=%GLIncludes% %AssimpIncludes% %ImguiIncludes% %stbIncludes% %glmIncludes%

REM Building win32_crystalflask.exe
IF EXIST gl3w.obj (
CALL :__START_TIME_MEASURE
echo Building win32_crystalflask.exe

cl %Win32Includes% %CommonCompilerFlags% ..\crystalflask\code\win32_crystalflask.cpp gl3w.obj %ImguiObjs% /Fewin32_crystalflask.exe -Fmwin32_crystalflask.map /link %CommonLinkerFlags% %AssimpLibs% crystalflask.res

CALL :__STOP_TIME_MEASURE
) ELSE (

CALL :__START_TIME_MEASURE
echo Building win32_crystalflask.exe

cl %Win32Includes% %CommonCompilerFlags% ..\crystalflask\code\win32_crystalflask.cpp %GLSrc% %ImguiSrc% /Fewin32_crystalflask.exe -Fmwin32_crystalflask.map /link %CommonLinkerFlags% %AssimpLibs% crystalflask.res

CALL :__STOP_TIME_MEASURE
)



popd

pushd ..\data
call ..\misc\build_number_incrementer.exe build.counter
popd








:__START_TIME_MEASURE
SET STARTTIME=%TIME%
SET STARTTIME=%STARTTIME: =0%
EXIT /B 0

:__STOP_TIME_MEASURE
SET ENDTIME=%TIME%
SET ENDTIME=%ENDTIME: =0%
SET /A STARTTIME=(1%STARTTIME:~0,2%-100)*360000 + (1%STARTTIME:~3,2%-100)*6000 + (1%STARTTIME:~6,2%-100)*100 + (1%STARTTIME:~9,2%-100)
SET /A ENDTIME=(1%ENDTIME:~0,2%-100)*360000 + (1%ENDTIME:~3,2%-100)*6000 + (1%ENDTIME:~6,2%-100)*100 + (1%ENDTIME:~9,2%-100)
SET /A DURATION=%ENDTIME%-%STARTTIME%
IF %DURATION% == 0 SET TIMEDIFF=00:00:00,00 && EXIT /B 0
IF %ENDTIME% LSS %STARTTIME% SET /A DURATION=%STARTTIME%-%ENDTIME%
SET /A DURATIONH=%DURATION% / 360000
SET /A DURATIONM=(%DURATION% - %DURATIONH%*360000) / 6000
SET /A DURATIONS=(%DURATION% - %DURATIONH%*360000 - %DURATIONM%*6000) / 100
SET /A DURATIONHS=(%DURATION% - %DURATIONH%*360000 - %DURATIONM%*6000 - %DURATIONS%*100)
IF %DURATIONH% LSS 10 SET DURATIONH=0%DURATIONH%
IF %DURATIONM% LSS 10 SET DURATIONM=0%DURATIONM%
IF %DURATIONS% LSS 10 SET DURATIONS=0%DURATIONS%
IF %DURATIONHS% LSS 10 SET DURATIONHS=0%DURATIONHS%
SET TIMEDIFF=%DURATIONM%m %DURATIONS%s %DURATIONHS%0ms
echo BUILD TIME: %TIMEDIFF%
EXIT /B 0


