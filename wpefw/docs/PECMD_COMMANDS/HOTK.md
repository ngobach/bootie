# HOTK

To set a hotkey of system level and associate a command with it.(.EXE or .CMD or .BAT),
supporting the internal commands of PECMD.

## Syntax

```text
HOTK [--delall] [?[.]RetVarName] [ModKey+]<KeyLetter|#VKCode>[,HKCmd]
```

## Parameters

  ☼ModKey+: 4 modifier keys (Alt,Ctrl,Shift and Win)can be indicated as a string,connected
  with "+".
- **KeyLetter**: Simple keys("a"~"z" or "0"~"9") can be indicated as letters directly,"A"~"Z"
  is indicated together with SHIFT,while other keys should be indicated with "#" followed by
  VKcode.It is allowed to directly use VK_Name(VK_can be absent) or the alternative string
  in the appendix,hex numbers supported.
- **Keycode**: consisting of "#" and the VKcode behind,see example 1 and example 2,hex number
  supported.
- **HKCMD**: a valid command supported by WinCMD. --del:to delete.

## Examples

```text
HOTK #255,EXEC PECMD.EXE SHUT or HOTK Ctrl + Alt + #36,EXEC PECMD.EXE or HOTK ?V #255
```

## Remarks

- Example 1 indicates calling SHUT to shut down the computer after pressing the power button
- This command cannot be used in the command line but in config file only.
- It is suggested abandoning PELOGON.The hotkeys are unlimited.
- The outcome of hotkeys registration is stored in "HKEY_LOCAL_MACHINE\SOFTWARE\PELOGON".
- The command of SHEL should be located behind HOTK and HIDE so that the process of
  explorer.exe comes into being and HOTK takes effect,or just kill the current process of
  explorer.exe so that it reruns automatically and thus HOTK takes effect.For the lastest
  version of PECMD,however,HOTK takes effect in real time and is ready for use at any time.
- Generally,only after loading SHELL with SHEL can hotkeys be registered by HOTK.
- 1:For the present version of PECMD,it is also applicable to a normal OS;
  2:It takes effect immediately.

