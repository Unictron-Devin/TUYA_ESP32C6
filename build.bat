@echo off
REM ESP32-C6 Tuya Project Interactive Menu
setlocal enabledelayedexpansion
set PORT=COM83

REM Change to project directory
cd /d "D:\User\Project\ESP32-C6_Project\TUYA_ESP32C6"

REM Set Python environment
set "IDF_PYTHON_ENV_PATH=D:\Espressif\python_env\idf5.4_py3.11_env"
set "PATH=%IDF_PYTHON_ENV_PATH%\Scripts;%PATH%"

:menu
cls
echo.
echo =====================================
echo   ESP32-C6 Tuya Build Menu
echo   Port: %PORT%
echo =====================================
echo.
echo   [1] Build Only
echo   [2] Flash Only  
echo   [3] Monitor
echo   [4] Build + Flash
echo   [5] Build + Flash + Monitor (FULL)
echo   [6] Clean Build
echo   [7] MenuConfig
echo   [8] Set Target
echo   [9] Exit
echo.
set /p choice=   Enter choice (1-9): 

if "%choice%"=="1" (
    cls
    echo [*] Building project...
    echo.
    call C:\Users\devin.huang\esp\v5.4.2\esp-idf\export.bat
    call idf.py build
    echo.
    pause
    goto menu
)

if "%choice%"=="2" (
    cls
    echo [*] Flashing to %PORT%...
    echo.
    call C:\Users\devin.huang\esp\v5.4.2\esp-idf\export.bat
    call idf.py -p %PORT% flash
    echo.
    pause
    goto menu
)

if "%choice%"=="3" (
    cls
    echo [*] Connecting to %PORT% monitor...
    echo [*] Press Ctrl+] to exit
    echo.
    call C:\Users\devin.huang\esp\v5.4.2\esp-idf\export.bat
    call idf.py -p %PORT% monitor
    goto menu
)

if "%choice%"=="4" (
    cls
    echo [*] Building and flashing to %PORT%...
    echo.
    call C:\Users\devin.huang\esp\v5.4.2\esp-idf\export.bat
    call idf.py -p %PORT% build flash
    echo.
    pause
    goto menu
)

if "%choice%"=="5" (
    cls
    echo [*] FULL BUILD: Build + Flash + Monitor to %PORT%...
    echo [*] Press Ctrl+] to exit monitor
    echo.
    call C:\Users\devin.huang\esp\v5.4.2\esp-idf\export.bat
    if errorlevel 1 (
        echo [ERROR] Failed to initialize ESP-IDF environment
        pause
        goto menu
    )
    call idf.py -p %PORT% build flash monitor
    if errorlevel 1 (
        echo [ERROR] Build or flash failed - check COM port
        pause
        goto menu
    )
    echo [OK] Build + Flash + Monitor completed
    pause
    goto menu
)

if "%choice%"=="6" (
    cls
    echo [*] Cleaning build directory...
    echo.
    call C:\Users\devin.huang\esp\v5.4.2\esp-idf\export.bat
    call idf.py fullclean
    echo [OK] Build cleaned
    echo.
    pause
    goto menu
)

if "%choice%"=="7" (
    cls
    echo [*] Opening menuconfig...
    echo.
    call C:\Users\devin.huang\esp\v5.4.2\esp-idf\export.bat
    call idf.py menuconfig
    goto menu
)

if "%choice%"=="8" (
    cls
    echo [*] Setting target to ESP32-C6...
    echo.
    call C:\Users\devin.huang\esp\v5.4.2\esp-idf\export.bat
    call idf.py set-target esp32c6
    echo [OK] Target set
    echo.
    pause
    goto menu
)

if "%choice%"=="9" (
    echo [*] Exiting...
    exit /b 0
)

echo [ERROR] Invalid choice
pause
goto menu

