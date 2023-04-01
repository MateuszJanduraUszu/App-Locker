App Locker
---

App Locker is a straightforward application that blocks the execution of specific
applications written in C++ 20. It can operate without administrator privileges,
but it requires them for the setup process.

Installation
---

1. Open `install.bat`.
2. Change `Your absolute path to applocker.exe` to path where `applocker.exe` is localized,
for eg. `C:\Users\Username\Desktop\App Locker\applocker.exe`.
3. Save.
4. Run as administrator (`sc.exe` requires administrator privileges).

Uninstallation
---

Just run `uninstall.bat` as administrator.

Usage
---

`dbmgr.exe` manages the list of locked applications. Below is a list of commands
that can be used with the `dbmgr.exe` utility:
* `--help` - Displays usage information for the application.
* `--lock=name` - Locks the specified application.
* `--unlock=name` - Unlocks the specified application.
* `--unlock-all` - Unlocks all locked applications.
* `--status=name` - Checks whether the specified application is currently locked.

How it works?
---

The App Locker application consists of two components - the App Locker Database
Manager (ALDM) and the App Locker Service (ALS). The ALDM maintains the list of locked
applications, while the ALS searches for and terminates any locked application processes.
Note that the ALS uses a directory watcher to receive updates on the list of locked
applications, making it safe to use the ALDM while the ALS is running.

Compatibility
---

I have tested it on Windows 11 22H2 22621.1413 and it worked well.
While I have not tested it on Windows 10, I expect it to work equally well on
that operating system.

Additional informations
---

Please note that locking system applications can cause errors and even break your
operating system. You should carefully consider the consequences before locking any
application. Additionally, please note that the App Locker Service refreshes every
3 seconds. As a result, locked applications may close at different times. This is
intentional and designed to avoid high CPU usage, especially on CPUs with fewer cores.