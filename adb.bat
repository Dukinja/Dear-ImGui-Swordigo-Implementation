adb logcat -c
adb logcat | Select-String -Pattern "MiniFeatureImGui|MiniNativeMain|MiniHookCore|AndroidRuntime|FATAL EXCEPTION|SIGSEGV|libmini|libGlossHook|libswordigo"
