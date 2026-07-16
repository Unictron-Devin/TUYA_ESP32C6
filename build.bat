@echo off
REM ESP32-C6 Tuya Project Build Script for Windows

setlocal enabledelayedexpansion

if "%1"=="" (
    set PORT=COM3
) else (
    set PORT=%1
)

echo ======================================
echo TUYA ESP32-C6 Build Script - Windows
echo ======================================
echo Target Port: %PORT%
echo.

REM Check if IDF_PATH is set
if "%IDF_PATH%"=="" (
    echo ERROR: IDF_PATH environment variable is not set
    echo Please set IDF_PATH to your ESP-IDF installation directory
    pause
    exit /b 1
)

REM Source ESP-IDF environment
call %IDF_PATH%\tools\idf_cmd_init.bat

:menu
cls
echo ======================================
echo TUYA ESP32-C6 Build Menu
echo ======================================
echo 1. Build
echo 2. Flash
echo 3. Monitor
echo 4. Build and Flash
echo 5. Build, Flash and Monitor
echo 6. Clean
echo 7. Menu Config
echo 8. Exit
echo.
set /p choice=Select option (1-8): 

if "%choice%"=="1" (
    echo Building project...
    call idf.py build
    pause
    goto menu
)
if "%choice%"=="2" (
    echo Flashing to device...
    call idf.py -p %PORT% flash
    pause
    goto menu
)
if "%choice%"=="3" (
    echo Monitoring serial output...
    call idf.py -p %PORT% monitor
    goto menu
)
if "%choice%"=="4" (
    echo Building and flashing...
    call idf.py -p %PORT% build flash
    pause
    goto menu
)
if "%choice%"=="5" (
    echo Building, flashing and monitoring...
    call idf.py -p %PORT% build flash monitor
    goto menu
)
if "%choice%"=="6" (
    echo Cleaning build files...
    call idf.py fullclean
    pause
    goto menu
)
if "%choice%"=="7" (
    echo Opening menuconfig...
    call idf.py menuconfig
    goto menu
)
if "%choice%"=="8" (
    echo Exiting...
    exit /b 0
)

echo Invalid option. Please try again.
pause
goto menu
