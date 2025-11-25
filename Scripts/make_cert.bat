@echo off
set CERT_NAME=TssShieldCert
set PFX_FILE=%CERT_NAME%.pfx
set PVK_FILE=%CERT_NAME%.pvk
set CER_FILE=%CERT_NAME%.cer

echo --- Creating Test Certificate ---
makecert -r -pe -n "CN=%CERT_NAME%" -ss PrivateCertStore -sv %PVK_FILE% %CER_FILE%

echo --- Creating PFX file ---
pvk2pfx -pvk %PVK_FILE% -spc %CER_FILE% -pfx %PFX_FILE%

echo --- Certificate generation complete ---
echo PFX file: %PFX_FILE%
echo CER file: %CER_FILE%
