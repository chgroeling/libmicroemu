@echo off


echo Activate virtual python and conan environment and start VSCode
CALL build\venv\Scripts\activate.bat
if %ERRORLEVEL% NEQ 0 (GOTO ScriptsError)

CALL build\Debug\generators\conanbuild.bat
if %ERRORLEVEL% NEQ 0 (GOTO ScriptsError)

rem no error occurred, jump to END
goto :END

:ScriptsError
(
  echo off
  rem COLOR 0C
  echo.
  echo ***************************************************
  echo * ERROR: %ERRORLEVEL%                   *
  echo ***************************************************
  echo.
  SET LAST_ERROR=%ERRORLEVEL%
  CALL build\Debug\generators\deactivate_conanbuild.bat
  CALL build\venv\Scripts\deactivate.bat
  pause
  exit /B %LAST_ERROR%
)

pause
:END