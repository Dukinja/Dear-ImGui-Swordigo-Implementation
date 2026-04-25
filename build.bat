@echo off
setlocal

REM ===== Configure local SDK/NDK paths =====
set "ANDROID_HOME=C:\Users\Owner\AppData\Local\Android\Sdk"
set "ANDROID_NDK_ROOT=D:\android\android-ndk-r27c"
set "ANDROID_NDK_HOME=%ANDROID_NDK_ROOT%"

REM Optional: set custom vanilla APK path as first arg.
REM Example:
REM   build-swmini-debug.bat "D:\apks\swordigo-1.4.12.apk"
set "VANILLA_ARG="
if not "%~1"=="" set "VANILLA_ARG=-Path=%~1"

echo Building SwMini debug APK...
echo ANDROID_HOME=%ANDROID_HOME%
echo ANDROID_NDK_ROOT=%ANDROID_NDK_ROOT%

if not exist "%ANDROID_HOME%\platform-tools\adb.exe" (
  echo ERROR: ANDROID_HOME seems invalid: %ANDROID_HOME%
  exit /b 1
)

if not exist "%ANDROID_NDK_ROOT%\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe" (
  echo ERROR: ANDROID_NDK_ROOT seems invalid: %ANDROID_NDK_ROOT%
  exit /b 1
)

call gradlew :app:assembleDebug %VANILLA_ARG%
if errorlevel 1 (
  echo.
  echo Build FAILED.
  exit /b %errorlevel%
)

echo.
echo Build succeeded:
echo   app\build\outputs\apk\debug\app-debug.apk
exit /b 0
