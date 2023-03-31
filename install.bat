:: install.bat

:: Copyright (c) Mateusz Jandura. All rights reserved.
:: SPDX-License-Identifier: Apache-2.0

@echo off
sc create "App Locker" start=auto binPath="Your absolute path to applocker.exe"
sc start "App Locker"