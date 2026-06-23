# MAIN

To initialize the desktop,take over Ctrl+Alt+Del and create a new process to fulfill the
functions of LOAD. --user:of user level.

## Syntax

```text
MAIN[*]  [--user] [--wait] [FilePath|CMD begin with TEAM or EXEC]
```

## Parameters

- **to specify the path of the config file to initialize.* indicates hiding the daemon**
  immediately.If the specified file does not exist and the #102 script is not blank,the #102
  script will be adopted.

## Examples

```text
MAIN %SystemRoot%\System32\PECMD.INI
```

## Remarks

- MAIN aims to create the process of LOAD,resident in the memory after executing LOAD.It
  will install the keyboard hook to hook the function of "ExitWindowsEx".
- It is suggested using PECMD MAIN directly instead of PELOGON.EXE.
- The command of LOAD will automatically quit the memory after it fulfills the function of
  loading files.                                                    Refer to LOAD for more.

