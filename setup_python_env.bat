@echo off

rem This batch file is intended as a helper script to setup a local developer 
rem build environment on windows.
rem It creates a virtual environment, activates the enviroment and installs
rem the requirements in it. 
rem If it fails, the virtual env will be deactivated.

if not exist "build" mkdir build

echo Create virtual python environment
echo Create virtual python environment %time% >> build\setup.log
python -m venv build/venv
if %ERRORLEVEL% NEQ 0 (GOTO ScriptsError)

echo Activate virtual environment and install dependencies
echo Activate virtual environment and install dependencies %time% >> build\setup.log
CALL build\venv\Scripts\activate.bat
if %ERRORLEVEL% NEQ 0 (GOTO ScriptsError)

python -m pip install -r requirements.txt
if %ERRORLEVEL% NEQ 0 (GOTO ScriptsError)


rem no error occured, jump to END
goto :END

:ScriptsError
(
  echo off
  rem COLOR 0C
  echo.
  echo ***************************************************
  echo * ERROR: %ERRORLEVEL%
  echo * Generating files failed                         *
  echo ***************************************************
  echo.
  SET LAST_ERROR=%ERRORLEVEL%
  CALL build\venv\Scripts\deactivate.bat
  exit /B %LAST_ERROR%
)

:END