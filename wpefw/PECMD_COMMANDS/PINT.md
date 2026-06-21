# PINT

To pin EXE or LNK file to "StartMenu" or "TaskBar".For NT5.x OS,only "StartMenu" is
supported.

## Syntax

```text
PINT [-TaskBand|-StartMenu] [*MenuDLLFileName,]<FileName>,[MenuName|MenuID|#StrID]
```

## Parameters

- **FileName**: the name of EXE and LNK files,containing its path and extension,supportive of
  environment variables interpretation;
- **MenuName**: string corresponding to the right click menu of EXE and LNK file,like "pinned to
  StartMenu(&U)" and "locked to TaskBar(&K)";
  It is suggested only keeping the front part without the hotkey mark "U)";
- **MenuID**: like "[-]StartMenu" or "TaskBar",the former indicates being pinned to "StartMenu"
  and the latter "TaskBar".[-]:to separate;
- **StrID**: number,the resource string ID in SHELL32.DLL for "pinned to StartMenu" and "locked
  to TaskBar".#5382:to separate from "StartMenu".

## Examples

```text
PINT %SystemRoot%\Regedit.EXE,StartMenu or PINT %SystemRoot%\Regedit.LNK,pinned to StartMenu
PINT %SystemRoot%\Regedit.EXE,TaskBar or PINT %SystemRoot%\Regedit.LNK,locked to TaskBar(&K)
PINT %WinDir%\System32\calc.exe,#5381 or PINT %WinDir%\System32\calc.LNK,#5386
```

## Remarks

- This command should be supported by the system function.In some simplified OS,XP and 2003
  OS,it is likely that the command cannot fulfill its function because of the simplification
  or defficiency of related system function;
- Only one of the three params "MenuName","MenuLabel" and "StrID" is employed.If all absent,
  it indicates pinning to "StartMenu".The three params vary with systems.It is suggested
  being indicated as "MenuName" of the system(like "pinned to StartMenu(&U));
- Other items of the system right click menu can be theoretically applied to "MenuName" to
  fulfill some functions,like using "Open(&O)" to open files.
- For win10,if pecmd is copied to its path.Dir\Exporer.exe,it will be put into use.Or else
  if there exists no such non-zero dword key as HKLM\SoftWare\PELONGON\USRCF\NOTMPEXPLORER,
  explorer.exe will be temporarily created and put into use.If there exists no such non-zero
  dword key as HKLM\SoftWare\PELONGON\USRCF\NODELAYEXPLORER,explorer.exe will delay its
  operation of TaskBand on demand.Such can be configured as the front part of corresponding
  MenuName of Win10 StartMenu:pecmd path.WIN10FromStartMenu,pecmd path.WIN10ToStartMenu,HKLM
  \SoftWare\PELONGON\USRCF\ WIN10FromStartMenu, WIN10ToStartMenu,WIN10FromTaskBand,
  WIN10ToTaskBand and PECMD 5381/5382 strings. Can place in path.USRCFG.INI

