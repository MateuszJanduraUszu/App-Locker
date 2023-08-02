:: build.bat

:: Copyright (c) Mateusz Jandura. All rights reserved.
:: SPDX-License-Identifier: Apache-2.0

@echo off
set platform_arch=%1
set compiler=%2

call :create_directory ".\dbmgr"
call :create_directory ".\dbmgr\%platform_arch%"
cd "dbmgr\%platform_arch%"
cmake -A %platform_arch% -G %compiler% ..\..
pause :: pause to see build logs

:create_directory
if not exist "%~1" (
	mkdir "%~1"
)