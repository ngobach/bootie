# HKEY

To set a hotkey of window or program level and associate a command with it.

## Syntax

```text
HKEY[$*]  [ModKey+]<KeyLetter|#VKCode>,<HKCMD>
```

## Parameters

  ☼"$" indicates hotkeys of program level(default).A hotkey cannot be associated with the
  same command as the one of another hotkey.* is of window level and the hotkey can be
  reassociated in different windows with unlimited commands.
- **ModKey+**: 4 modifier keys (Alt,Ctrl,Shift and Win)can be indicated as a string,connected
  with "+".
- **KeyLetter**: Simple keys("a"~"z" or "0"~"9") can be indicated directly."A"~"Z" is
  indicated together with SHIFT.It is allowed to directly use VK_Name(VK_can be absent) or
  the alternative string in the appendix,hex numbers supported.
- **Keycode**: consisting of "#" and the VKcode behind,see example 1 and example 2,hex number
  supported.
- **HKCMD**: a valid command supported by PECMD.

## Examples

```text
HKEY #255,SHUT R or HKEY Ctrl+Alt+#0x41,DISP W800H600B16F75 or HKEY Ctrl+Alt+D,TEMP @Delete
```

## Remarks

- This command can be located beyond _SUB and _END(with $) to achieve certain functions.
  Refer to _SUB and CALL.
- Example 1 indicates calling SHUT to reboot after pressing the power button;
  Example 2 indicates calling DISP to set the display mode after pressing Ctrl+Alt+A;
- Example 3 indicates calling TEMP to clear temporary directory after pressing Ctrl+Alt+D.
  Notice that the set hotkeys can not conflict with the ones of other programs.

