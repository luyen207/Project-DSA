@echo off
rem Bien dich ShopPro tren Windows (CHUA duoc kiem thu tren Windows - xem Backend\README.md).
rem Yeu cau: CMake, Visual Studio Build Tools (hoac MSYS2), OpenSSL (vcpkg: vcpkg install openssl:x64-windows).
rem Neu dung vcpkg, dat bien moi truong VCPKG_ROOT truoc khi chay file nay.
cd /d "%~dp0.."
if defined VCPKG_ROOT (
  cmake -S Backend -B build -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
) else (
  cmake -S Backend -B build
)
if errorlevel 1 exit /b 1
cmake --build build --config Release --parallel
if errorlevel 1 exit /b 1
echo.
echo Xong. Chay may chu bang: scripts\run.bat
