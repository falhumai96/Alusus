@echo off
REM We have migrated the build scripts to Python 3.4+ (we will not support any Python version lower than that).
set SCRIPT_DIR=%~dp0
python -m pip install "%SCRIPT_DIR%\BuildSrc\requirements.txt" &&^
python "%SCRIPT_DIR%\BuildSrc\build.py" %*
exit /b %errorlevel%
