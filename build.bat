@echo off
setlocal

for %%I in ("%~dp0.") do set "ROOT=%%~fI"
set "BUILD=%ROOT%\build"
set "LSPF=%ROOT%\lspf"
set "DEBUG_EXE=%BUILD%\Debug\tmlang.exe"
set "RELEASE_EXE=%BUILD%\Release\tmlang.exe"

rem First argument is the command; everything after it is forwarded to tmlang.
set "CMD=%~1"
set "ARGS="
:collect_args
shift
if "%~1"=="" goto :dispatch
set ARGS=%ARGS% %1
goto :collect_args

:dispatch
if "%CMD%"=="init"             goto :init
if "%CMD%"=="clean"            goto :clean
if "%CMD%"=="compile"          goto :compile
if "%CMD%"=="run"              goto :run
if "%CMD%"=="compile-release"  goto :compile_release
if "%CMD%"=="run-release"      goto :run_release
if "%CMD%"=="restart"          goto :restart
if "%CMD%"=="lsp"              goto :lsp

echo Usage:
echo   build.bat init
echo   build.bat clean
echo   build.bat compile
echo   build.bat run [args...]
echo   build.bat compile-release
echo   build.bat run-release [args...]
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
"%DEBUG_EXE%"%ARGS%
exit /b %errorlevel%

:compile_release
if not exist "%BUILD%" call :init || exit /b 1
cmake --build "%BUILD%" --config Release --parallel
exit /b %errorlevel%

:run_release
call :compile_release || exit /b 1
"%RELEASE_EXE%"%ARGS%
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
