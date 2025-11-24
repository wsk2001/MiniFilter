@echo off
set SERVICE_NAME=TssShield
set AGENT_EXE=C:\TssShield.exe
set DRIVER_SYS=C:\Windows\System32\drivers\TssShield.sys
set DRIVER_INF=C:\Windows\inf\TssShield.inf
set DRIVER_CAT=C:\Windows\System32\catroot\{F750E6C3-38EE-11D1-85E5-00C04FC295EE}\TssShield.cat

echo --- Stopping Services ---
sc stop %SERVICE_NAME%
net stop TssShield

echo --- Uninstalling Agent Service ---
%AGENT_EXE% --uninstall

echo --- Uninstalling Driver ---
for /f "tokens=2" %%i in ('pnputil /enum-drivers ^| findstr /c:"TssShield.inf"') do (
    pnputil /delete-driver %%i /uninstall /force
)

echo --- Deleting files ---
del %AGENT_EXE%
del %DRIVER_SYS%
del %DRIVER_INF%
del %DRIVER_CAT%

echo --- Uninstallation complete ---
