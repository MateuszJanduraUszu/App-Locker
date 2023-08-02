:: uninstall.bat

:: Copyright (c) Mateusz Jandura. All rights reserved.
:: SPDX-License-Identifier: Apache-2.0

@echo off
sc stop "App Locker"
sc delete "App Locker"