@echo off
setlocal

for %%I in ("%~dp0.") do set "ROOT=%%~fI"
set "BUILD=%ROOT%\build"
set "LSPF=%ROOT%\lspf"
set "DEBUG_EXE=%BUILD%\Debug\tmlang.exe"
set "RELEASE_EXE=%BUILD%\Release\tmlang.exe"

if "%~1"=="init"             goto :init
if "%~1"=="clean"            goto :clean
if "%~1"=="compile"          goto :compile
if "%~1"=="run"              goto :run
if "%~1"=="compile-release"  goto :compile_release
if "%~1"=="run-release"      goto :run_release
if "%~1"=="restart"          goto :restart
if "%~1"=="lsp"              goto :lsp

echo Usage:
echo   build.bat init
echo   build.bat clean
echo   build.bat compile
echo   build.bat run
echo   build.bat compile-release
echo   build.bat run-release
echo   build.bat restart
exit /b 1

:init
if not exist "%BUILD%" mkdir "%BUILD%"
cmake -S "%ROOT%" -B "%BUILD%"
exit /b %errorlevel%

:clean
if exist "%BUILD%" rmdir /S /Q "%BUILD%"
mkdir "%BUILD%"
exit /b %errorlevel%

:compile
if not exist "%BUILD%" call :init || exit /b 1
cmake --build "%BUILD%" --config Debug --parallel
exit /b %errorlevel%

:run
call :compile || exit /b 1
"%DEBUG_EXE%"
exit /b %errorlevel%

:compile_release
if not exist "%BUILD%" call :init || exit /b 1
cmake --build "%BUILD%" --config Release --parallel
exit /b %errorlevel%

:run_release
call :compile_release || exit /b 1
"%RELEASE_EXE%"
exit /b %errorlevel%

:lsp
if exist "%LSPF%" rmdir /S /Q "%LSPF%"
mkdir "%LSPF%"
cmake -S "%ROOT%" -B "%LSPF%" -G "Ninja Multi-Config" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
move /Y "%LSPF%\compile_commands.json" "%ROOT%"
rmdir /S /Q "%LSPF%"
exit /b %errorlevel%

:restart
call :lsp || exit /b 1
call :clean || exit /b 1
call :init || exit /b 1
call :run || exit /b 1
exit /b %errorlevel%
