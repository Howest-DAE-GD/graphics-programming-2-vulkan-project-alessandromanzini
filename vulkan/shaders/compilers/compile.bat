@echo off
setlocal enabledelayedexpansion

:: Set the path to glslc
set "GLSLC=%VULKAN_SDK%\bin\glslc.exe"

:: Initialize variables
set "OUTPUT="
set "INPUTS="
set "ARGS="

:: Parse arguments
:parse_args
if "%~1"=="" goto done_parsing
if "%~1"=="--version" (
    "%GLSLC%" --version
    exit /b
) else if "%~1"=="-o" (
    shift
    if "%~1"=="" (
        echo Expected an argument for -o flag!
        exit /b 1
    )
    set "OUTPUT=%~1"
) else if "%~1"=="-i" (
    shift
    :collect_inputs
    if "%~1"=="" goto parse_args
    echo %~1 | findstr "^-" >nul
    if not errorlevel 1 goto parse_args
    set "INPUTS=!INPUTS! %~1"
    shift
    goto collect_inputs
) else (
    echo %~1 | findstr "^-" >nul
    if errorlevel 1 (
        echo Unknown argument: %~1
        exit /b 1
    )
    set "ARGS=!ARGS! %~1"
)
shift
goto parse_args
:done_parsing

if "%OUTPUT%"=="" (
    echo Output folder not specified!
    exit /b 1
)

:: Create the output directory
if not exist "%OUTPUT%" (
    mkdir "%OUTPUT%"
)

:: Loop through input files and compile
for %%I in (%INPUTS%) do (
    set "INFILE=%%~I"
    set "OUTFILE=%OUTPUT%\%%~nxI.spv"
    echo "%GLSLC%" !INFILE! !ARGS! -o "!OUTFILE!"
    "%GLSLC%" !INFILE! !ARGS! -o "!OUTFILE!"
)

endlocal