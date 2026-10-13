@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -STA -File "%~dp0Install-LuaCEF.ps1" %*
set "LUACEF_INSTALL_EXIT=%ERRORLEVEL%"
pause
exit /b %LUACEF_INSTALL_EXIT%
