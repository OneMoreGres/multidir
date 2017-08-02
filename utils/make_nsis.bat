
set SELF_PATH=%~dp0
call %SELF_PATH%\make_win.bat
if %errorlevel% neq 0 exit /b %errorlevel%


mkdir content
mkdir content\app
mkdir content\app\translations
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
echo !define ARCH "%ARCH%" >> content\defines.nsi

copy /Y %SELF_PATH%\..\translations\*.qm content\app\translations
copy /Y %SELF_PATH%\multidir.nsi content
copy /Y %SELF_PATH%\..\LICENSE.md content\LICENSE_en.md
copy /Y %SELF_PATH%\LICENSE_ru.md content\LICENSE_ru.md
TYPE %SELF_PATH%\Changelog_en.txt | MORE /P > content\Changelog_en.txt
TYPE %SELF_PATH%\Changelog_ru.txt | MORE /P > content\Changelog_ru.txt
copy /Y %SELF_PATH%\..\icons\icon.ico content

makensis.exe content/multidir.nsi

copy /b /Y content\multidir-*.exe multidir-installer-%ARCH%.exe

