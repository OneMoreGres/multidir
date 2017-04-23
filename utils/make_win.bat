
set SELF_PATH=%~dp0
call %SELF_PATH%\win_env.bat

qmake %SELF_PATH%\..\
nmake
lrelease %SELF_PATH%\..\multidir.pro
