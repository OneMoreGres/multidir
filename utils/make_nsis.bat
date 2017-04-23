
set SELF_PATH=%~dp0
call %SELF_PATH%\make_win.bat


mkdir content
mkdir content\app
mkdir content\translations
copy /Y release\multidir.exe content\app

windeployqt --release content\app\multidir.exe

::cd content\app
::rd /Q /S iconengines
::rd /Q /S imageformats
::copy /Y translations\qt_ru.qm .
::rd /Q /S translations
::del d3d*
::del libEGL.dll
::del libGLESV2.dll
::del opengl32sw.dll
::del Qt5Svg.dll
::del vcredist*
::cd ..
::cd ..

set /p VERSION=<%SELF_PATH%\..\version
del content\defines.nsi
echo !undef VERSION >> content\defines.nsi
echo !define VERSION "%VERSION%" >> content\defines.nsi

copy /Y %SELF_PATH%\..\translations\*.qm content\app\translations
copy /Y %SELF_PATH%\multidir.nsi content
copy /Y %SELF_PATH%\..\LICENSE.md content\LICENSE_en.md
copy /Y %SELF_PATH%\LICENSE_ru.md content\LICENSE_ru.md
copy /Y %SELF_PATH%\Changelog* content
copy /Y %SELF_PATH%\..\icons\icon.ico content

makensis.exe content/multidir.nsi

copy /b /Y content\multidir-*.exe multidir-latest.exe

