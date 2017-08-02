
set SELF_PATH=%~dp0
call %SELF_PATH%\win_env.bat

lrelease %SELF_PATH%\..\multidir.pro
qmake %SELF_PATH%\..\
nmake

mkdir tests
cd tests
qmake %SELF_PATH%\..\tests\
nmake
cd ..
tests\release\tests.exe
