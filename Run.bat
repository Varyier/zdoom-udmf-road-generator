
@echo off

set APP_PATH=Release\road-gen.exe

if exist "x64\Release\road-gen.exe" (set APP_PATH=x64\Release\road-gen.exe)
if not exist %APP_PATH% ( goto error )

%APP_PATH% res\data.txt roads.wad -config road-config.txt
exit /b 0

:error
echo !!! ERROR: cannot find program executable - build it first
exit /b 1