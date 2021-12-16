set OutDir=%1

if exist %OutDir%Include\winrt\ (
    del /s /q %OutDir%Include\winrt\ || exit /b 1
)

REM Copy the winrt folder. 
xcopy /E /I "Generated Files\winrt\WinrtServer*" %OutDir%Include\winrt\ || exit /b 1
