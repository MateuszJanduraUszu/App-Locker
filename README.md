# App Locker

App Locker is a straightforward application that blocks the execution of specific
applications written in C++17. It can operate without administrator privileges,
but it requires them for the setup process.

## Build

To successfully build the project, follow these steps:

1. Ensure that you have CMake and a compiler known to CMake properly installed.
2. Clone the repository using the following command:

```bat
git clone https://github.com/MateuszJanduraUszu/App-Locker.git
```

3. Build the `applocker` executable:

```bat
cd build\scripts\applocker
build.bat {x64|Win32} "{Compiler}"
```

4. Build the `dbmgr` executable:

```bat
cd build\scripts\dbmgr
build.bat {x64|Win32} "{Compiler}"
```

These steps will help you compile the project's executables using the specified
platform architecture and compiler.

## Installation

Run `install.bat` as administrator.

## Uninstallation

Run `uninstall.bat` as administrator.

## Usage

`dbmgr.exe` manages the list of locked applications. Below is a list of commands
that can be used with the `dbmgr.exe` utility:
* `--help` - Displays usage information for the application.
* `--lock=name` - Locks the specified application.
* `--unlock=name` - Unlocks the specified application.
* `--unlock-all` - Unlocks all locked applications.
* `--status=name` - Checks whether the specified application is currently locked.

## Examples

- To lock an application:

```bat
dbmgr.exe --lock=Notepad.exe
```

- To unlock an application:

```bat
dbmgr.exe --unlock=Notepad.exe
```

- To unlock all locked applications

```bat
dbmgr.exe --unlock-all
```

- To check if an application is locked:

```bat
dbmgr.exe --status=Notepad.exe
```

## How it works

The App Locker application consists of two components - the App Locker Database
Manager (ALDM) and the App Locker Service (ALS). The ALDM maintains the list of locked
applications, while the ALS searches for and terminates any locked application processes.
Note that the ALS uses a directory watcher to receive updates on the list of locked
applications, making it safe to use the ALDM while the ALS is running.

## Compatibility

I have tested it on Windows 11 22H2 22621.1413 and it worked well.
While I have not tested it on Windows 10, I expect it to work equally well on
that operating system.

## Additional informations

Please note that locking system applications can cause errors and even break your
operating system. You should carefully consider the consequences before locking any
application.

## License

Copyright © Mateusz Jandura.

SPDX-License-Identifier: Apache-2.0