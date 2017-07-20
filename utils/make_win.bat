
set SELF_PATH=%~dp0
call %SELF_PATH%\win_env.bat

lrelease %SELF_PATH%\..\multidir.pro
qmake CONFIG+=x86_64 %SELF_PATH%\..\
nmake
