:: команды скопированы из idf_cmd_init.bat
GitWCRev . ./main/version_svn.fmt ./main/version_svn.h
@echo off
::set "IDF_PYTHON_DIR="C:\Users\A\AppData\Local\Programs\Python\Python37\""
set "IDF_PYTHON_DIR="%LOCALAPPDATA%\Programs\Python\Python37\""
set "IDF_GIT_DIR="C:\Program Files\Git\cmd\""
set "IDF_TOOLS_JSON_PATH=%IDF_PATH%\tools\tools.json"
set "PATH=%IDF_PYTHON_DIR%;%IDF_GIT_DIR%;%PATH%"
set "IDF_TOOLS_PY_PATH=%IDF_PATH%\tools\idf_tools.py"
set "IDF_TOOLS_EXPORTS_FILE=%TEMP%\idf_export_vars.tmp"
python.exe "%IDF_TOOLS_PY_PATH%" --tools-json "%IDF_TOOLS_JSON_PATH%" export --format key-value >"%IDF_TOOLS_EXPORTS_FILE%"
if %errorlevel% neq 0 goto :end
for /f "usebackq tokens=1,2 eol=# delims==" %%a in ("%IDF_TOOLS_EXPORTS_FILE%") do (
      call set "%%a=%%b"
    )
call set PATH_ADDITIONS=%%PATH:%OLD_PATH%=%%
python.exe %IDF_PATH%\tools\check_python_dependencies.py

idf.py build
