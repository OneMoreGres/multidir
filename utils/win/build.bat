::setup
set SELF_PATH=%~dp0
call %SELF_PATH%\env.bat
set ROOT=%SELF_PATH%\..\..


::build
lrelease %ROOT%\multidir.pro
qmake %ROOT%
nmake
if %errorlevel% neq 0 exit /b %errorlevel%


::test
mkdir tests
cd tests
qmake %ROOT%\tests\
nmake
cd ..
tests\release\tests.exe
if %errorlevel% neq 0 exit /b %errorlevel%


::pack
mkdir content
mkdir content\app
mkdir content\app\translations
copy /Y release\multidir.exe content\app

windeployqt --release content\app\multidir.exe

set /p VERSION=<%ROOT%\version
del content\defines.nsi
echo !undef VERSION >> content\defines.nsi
echo !define VERSION "%VERSION%" >> content\defines.nsi
echo !define ARCH "%ARCH%" >> content\defines.nsi

copy /Y %ROOT%\translations\*.qm content\app\translations
copy /Y %SELF_PATH%\multidir.nsi content
copy /Y %ROOT%\LICENSE.md content\LICENSE_en.md
copy /Y %ROOT%\utils\LICENSE_ru.md content\LICENSE_ru.md
TYPE %ROOT%\utils\Changelog_en.txt | MORE /P > content\Changelog_en.txt
TYPE %ROOT%\utils\Changelog_ru.txt | MORE /P > content\Changelog_ru.txt
copy /Y %ROOT%\icons\icon.ico content

makensis.exe content/multidir.nsi

copy /b /Y content\multidir-*.exe multidir-installer-%VERSION%-%ARCH%.exe

