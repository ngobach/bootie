# INIT

To carry out the basic initialization,register windows shell,initialize users directory and
environment variables,install keyboard hook and create following directories:
Favorites                                                                    Programs
Desktop                                                                      SendTo
StartMenu                                                                    Personal
Startup                                                                      QuickLaunch

## Syntax

```text
INIT [OptionList][,TimeWait][,USBStartDrvLetter]
```

## Parameters

- **OptionList**: 4 options supported,consisting of "C","I","K" and "U",which can be used in
  indefinite items,regardless of their sequence.
  Ѻ"C":to store the drive letter for CDROM in an environment variable;
  Ѻ"I":to install some functions of PECMD to the menu of tray icon;
  Ѻ"K":to install low-level keyboard hook when executing INIT,otherwise, only after loading
  SHELL will it be installed;
  Ѻ"U":to check the insertion of USB removable hard disk,assign drive letters automatically
  and offer the function of plug-and-play of it.
- **TimeWait**: number,indicates the time to wait(ms) from the insertion of the removable hard
  disk to its being ready.If the number is too small,the assignment of drive letter is
  likely to fail.
- **USBStartDrvLetter**: the starting drive letter for USB devices,U by default.

## Examples

```text
INIT or INIT C or INIT CU or INIT CIK or INIT CIKU or INIT IU,3000
```

## Remarks

- After INIT is executed and the specified SHELL is loaded by SHEL,a minimized WinPE can be
  launched immediately.
- To ensure that there is sufficient space available in the partition containing
  %USERPROFILE%,or INIT will not fulfill its task.
- With the option "C",the drive letter for CDROM will be stored in an environment varialbe
  beginning with CDROM(It takes effect only after refreshing with the command of ENVI
  without any param).
- With the option "I",an icon menu,integrated with some commonly used functions,will appear
  in the tray.The custom personnel of WinPE can modify these functions of the menu.
- With the option "K",it indicates installing the keyboard hook immediately to take over
  Ctrl+Alt+Del to summon the task manager.
- With the option "U",the command will achieve the function of plug-and-play for the
  removable hard disk,which can be removed by "EJEC U-" or from the tray menu thence.
- If "TimeWait" is absent,a default value(3000) will be adopted; If the option "U" is absent
  ,so shall "TimeWait".
- It is not suggested for INIT in publicly-issued WinPE to carry the option "K".This command
  is only applicable to the config file not to command line.

