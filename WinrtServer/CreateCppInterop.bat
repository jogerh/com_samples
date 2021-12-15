set OutDir=%1
if not exist %OutDir%Include\WinrtServer (
    mkdir %OutDir%Include\WinrtServer || exit /b 1
)
winmdidl %OutDir%\WinrtServer.winmd /outdir:%OutDir%Include\WinrtServer\ || exit \b 1
pushd %OutDir%Include\WinrtServer || exit /b 1
midlrt WinrtServer.idl /metadata_dir C:\Windows\System32\WinMetadata || exit /b 1