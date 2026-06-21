# SHEL

To load the specified SHELL and lock it.  -user:forced to coordinate with MAIN -user.
-sys:forced to be the system Shell.

## Syntax

```text
SHEL [-user|-sys] [-shel:"AutoCMD"]
     <FileName(including path)|CMD beginning with TEAM or EXEC>[,PWBASEStr] [,TimesRetry]
```

## Parameters

- **SHELL FileName(including path.Environment variables can be used)**
- **The password should be letters and numbers,case sensitive.**
- **If PWBASEStr is absent,the automatic login will be initiated.The default TimesRetry is 3.**

## Examples

```text
SHEL %SystemRoot%\EXPLORER.EXE
```

## Remarks

- The function of this command resembles EXEC $.Codes for shutdown in the process can be
  modified(to hook the function of "ExitWindowsEx").
- This command is also capable of locking SHELL and automatically loading SHELL when it is
  killed.
- It is suggested using this command with HIDE so as to achieve a better effect.
- This command is not applicable to the command line but to config file only.
- SHEL should be placed behind HOTK and HIDE.

