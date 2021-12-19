$ErrorActionPreference = "Stop" # Don't continue script when a line fails
$WarningPreference = "SilentlyContinue"

$mydir = Split-Path $MyInvocation.MyCommand.Path -Parent

Push-Location $mydir

py -3.9 -m venv ..\Build\buildenv
& "..\Build\buildenv\Scripts\Activate.ps1"
python -m pip install -r requirements.txt --disable-pip-version-check

# In C++ projects we do it automatically, but for python server handcrafted generation needed 
midl RoyalPython.idl
python PyComServer.py -regserver

Pop-Location
