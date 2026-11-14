@echo off
rem Chay ShopPro tren Windows. Tham so duoc chuyen cho shoppro_api (vi du: scripts\run.bat --port 9000)
cd /d "%~dp0.."
if exist build\Release\shoppro_api.exe (
  build\Release\shoppro_api.exe %*
) else if exist build\shoppro_api.exe (
  build\shoppro_api.exe %*
) else (
  echo Chua co shoppro_api.exe. Hay chay scripts\build.bat truoc.
  exit /b 1
)
