@echo off

:: ȡ�õ�ǰ·��
SET CUR_PATH="%~dp0"

:: ��ȡ����ԱȨ��
:: if not "%1"=="am_admin" (powershell start -verb runas '%0' am_admin & exit /b)

:: ɾ��֮ǰ��link�汾
if exist %CUR_PATH%output\vs2017\Cfgs\ (
    rd /s/q %CUR_PATH%output\vs2017\Cfgs\
)

:: -------------------------------------- �������� -------------------------------------------
:: �����Զ��������ô��뼰����
REM call :relink_dir %CUR_PATH%config\csharp\client\pc %CUR_PATH%client\trunk\ClientAssembly\GameBusiness\src\Com\Config\Cfg
call :relink_dir %CUR_PATH%Service\Cfgs %CUR_PATH%output\vs2017\Cfgs

:: ------------------------------------ ������������ -----------------------------------------
echo Done!
ping -n 1 -w 1618 11.11.11.11 > nul
exit 0

:: -------------------------------------- �������� -------------------------------------------
:: relinkĿ¼��������
:relink_dir
if exist %2 (
    rd /s/q %2
)
mklink /d %2 %1
goto EOF

:: relink�ļ���������
:relink_file
if exist %2 (
    del /q %2
)
mklink %2 %1
goto EOF


::remove_dir ɾ��Ŀ¼
:remove_dir
set DelDir=%1
if exist %delDir% (
    rd /s/q %delDir%
)
goto EOF
:: ------------------------------------ ������������ -----------------------------------------

:EOF

