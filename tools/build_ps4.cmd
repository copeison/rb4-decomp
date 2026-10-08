@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_ps4.ps1" %*
exit /b %ERRORLEVEL%
