@echo off
:: Variables
set ADB_PATH="C:\Users\Owner\AppData\Local\Android\Sdk\platform-tools\adb.exe"
set SOURCE="C:\Users\Owner\Documents\SwMini\app\build\outputs\apk\debug\app-debug.apk"
set DEST="/sdcard/Duke/swmodding/"

echo --- Starting Transfer ---
%ADB_PATH% push %SOURCE% %DEST%

echo.
echo --- Installing APK ---
:: -r: reinstall (keep data)
:: -t: allow test packages
%ADB_PATH% install -r -t %SOURCE%

echo.
echo Done!
pause