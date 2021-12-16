set OutDir=%1

if exist %OutDir%Include\WinrtServer\winrt\ (
    del /s /q %OutDir%Include\WinrtServer\winrt\ || exit /b 1
)

REM Copy the primary WinrtServer.h. This is a workaround for include path issues in generated files
xcopy /I "Generated Files\winrt\WinrtServer.h" %OutDir%Include\WinrtServer\

REM Copy the winrt folder. 
xcopy /E /I "Generated Files\winrt\WinrtServer*" %OutDir%Include\WinrtServer\winrt\ || exit /b 1

