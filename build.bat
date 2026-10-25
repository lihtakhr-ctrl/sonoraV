@echo off
cd /d "%~dp0"
chcp 65001 >nul
title R5R Cheat - build

echo ==========================================
echo   R5R Cheat  build
echo ==========================================
echo.

echo [1/4] CMake を探しています...
where cmake >nul 2>nul
if errorlevel 1 goto NO_CMAKE

echo [2/4] 構成しています...
cmake -S . -B build -A x64
if errorlevel 1 goto CMAKE_FAIL

echo [3/4] ビルドしています... 初回は数分かかります
cmake --build build --config Release
if errorlevel 1 goto BUILD_FAIL

echo [4/4] 成功しました
echo     build\Release\R5RCheat.exe
goto DONE

:NO_CMAKE
echo.
echo  *** CMake が見つかりません ***
echo.
echo  Visual Studio Installer を開いて
echo    [変更] - [個別のコンポーネント]
echo    "C++ CMake tools for Windows" にチェック
echo  を入れてインストールしてください。
echo.
goto DONE

:CMAKE_FAIL
echo.
echo  *** CMake の構成に失敗しました ***
echo.
echo  "Visual Studio が見つかりません" と出た場合は
echo  [C++ によるデスクトップ開発] が未インストールです。
echo.
goto DONE

:BUILD_FAIL
echo.
echo  *** ビルドに失敗しました ***
echo  上のエラーを確認してください。
echo.
goto DONE

:DONE
echo ==========================================
echo  この画面はキーを押すまで閉じません
echo ==========================================
pause