set OutDir=%1
regsvr32 /s %OutDir%AtlServer.dll || exit /b 1
regsvr32 /s %OutDir%AtlFreeServer.dll || exit /b 1
regsvr32 /s %OutDir%ManagedServer\ManagedServer.comhost.dll || exit /b 1

powershell -File ..\PyComServer\RegPyComServer.ps1 || exit /b 1
