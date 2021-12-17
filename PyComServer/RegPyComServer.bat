setlocal
pushd %~dp0

@echo on

py -3 -m venv ..\Build\buildenv || exit /b 1
call ..\Build\buildenv\Scripts\Activate.bat || exit /b 1

python -m pip install -r requirements.txt || exit /b 1

:: In C++ projects we do it automatically, but for python server handcrafted generation needed 
midl RoyalPython.idl || exit /b 1
python PyComServer.py -regserver || exit /b 1

popd
