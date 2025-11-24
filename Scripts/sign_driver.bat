@echo off
set CERT_NAME=TssShieldCert
set PFX_FILE=%CERT_NAME%.pfx
set DRIVER_FILE=../Driver/x64/Release/TssShield.sys
set INF_FILE=../Driver/TssShield.inf
set CAT_FILE=../Driver/x64/Release/TssShield.cat

echo --- Signing the driver file ---
signtool sign /f %PFX_FILE% /t http://timestamp.digicert.com %DRIVER_FILE%

echo --- Creating CAT file ---
inf2cat /driver:.. /os:10_X64

echo --- Signing the CAT file ---
signtool sign /f %PFX_FILE% /t http://timestamp.digicert.com %CAT_FILE%

echo --- Driver signing complete ---
