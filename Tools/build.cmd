@echo off
setlocal enabledelayedexpansion

REM We have migrated the build scripts to Python 3.4+ (we will not support any Python version lower than that).
set SCRIPT_DIR=%~dp0

set args=%1
shift
:start
if [!%1] == [] goto done
set args=!args! %1
shift
goto start

:done

if "!ALUSUS_PYTHON_EXECUTABLE!"== "" (
    set PYTHON_EXECUTABLE=python.exe
) else (
    set PYTHON_EXECUTABLE=!ALUSUS_PYTHON_EXECUTABLE!
)

"!PYTHON_EXECUTABLE!" "!SCRIPT_DIR!\BuildSrc\install_deps.py" &&^
"!PYTHON_EXECUTABLE!" "!SCRIPT_DIR!\BuildSrc\build.py" !args!
exit /b !errorlevel!
