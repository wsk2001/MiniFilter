@echo off
set AGENT_EXE=../Agent/x64/Release/TssShield.exe
set DRIVER_SYS=../Driver/x64/Release/TssShield.sys
set DRIVER_INF=../Driver/TssShield.inf
set DRIVER_CAT=../Driver/x64/Release/TssShield.cat
set SERVICE_NAME=TssShield

echo --- Copying files ---
copy %AGENT_EXE% C:\TssShield.exe
copy %DRIVER_SYS% C:\Windows\System32\drivers\TssShield.sys
copy %DRIVER_INF% C:\Windows\inf\TssShield.inf
copy %DRIVER_CAT% C:\Windows\System32\catroot\{F750E6C3-38EE-11D1-85E5-00C04FC295EE}\TssShield.cat

echo --- Installing Agent Service ---
C:\TssShield.exe --install

echo --- Installing Driver ---
pnputil /add-driver C:\Windows\inf\TssShield.inf /install

echo --- Starting Services ---
sc start %SERVICE_NAME%
net start TssShield

echo --- Installation complete ---
