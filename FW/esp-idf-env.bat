@echo off

set ROOT_PATH=%~dp0
set SDK_PATH=%~dp0\3rdparty\esp-idf

@REM 更换下载地址
set IDF_GITHUB_ASSETS=dl.espressif.com/github_assets

@REM 尝试更新git源代码
echo ----------------尝试下载源代码!-------------------
git submodule update --init --recursive --jobs 8 2>nul  
if not exist "%SDK_PATH%\install.bat"  ( echo 请下载包含子模块的完整源代码! && goto :error_pause )
if not exist "%SDK_PATH%\export.bat" ( echo 请下载包含子模块的完整源代码! && goto :error_pause )

pushd %SDK_PATH%

@REM 安装工具
echo -----------------安装工具！----------------------
cmd /c  .\install.bat
if not ERRORLEVEL 0  ( popd && echo 安装工具失败! && goto :error_pause )

@REM 导出工具
echo -----------------导出工具！----------------------
call .\export.bat
if not ERRORLEVEL 0  ( popd && echo 导出工具失败! && goto :error_pause )

popd



cmd.exe

goto :eof
:error_pause
pause
goto :eof