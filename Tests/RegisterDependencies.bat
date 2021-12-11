set OutDir=%1
regsvr32 /s %OutDir%AtlServer.dll || exit /b 1
regsvr32 /s %OutDir%AtlFreeServer.dll || exit /b 1