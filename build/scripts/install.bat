:: install.bat

:: Copyright (c) Mateusz Jandura. All rights reserved.
:: SPDX-License-Identifier: Apache-2.0

@echo off
sc create "App Locker" start=auto binPath="%~dp0\applocker.exe"
sc start "App Locker"