# MENU

To add a menu item to the user-definied menu of the tray icon.-bar:window menu(indispensable
for the first command of MENU,optional for the subsequent ones);
-sub:Params behind belong to a submenu(_SUB).

## Syntax

```text
MENU [-bar] [-sub] <Name>,[Title],[Event],[Status][,BMP]
```

## Parameters

- **Name**: string,except the dividing line,it should be unique,not the same as those of other
  controls and environment variables.
  If Name or the first character of it is "-",it indicates a dividing line,or it is a normal
  menu item.
- **Title**: characters on the menu item to describe the functions or instructions of it.
- **Event**: the command to execute when the menu item is clicked.It should be a valid command
  supported by PECMD.*** indicates quitting the tray icon.
- **Status**: 0 by default,indicating a normal menu,- or ±1 indicates being disabled. 2:dividing
  line 4:checked 8:to hide. 0x20:subfield

## Examples

```text
MENU Menu2,definition 1024X768 color 32 bit RefRate 85,DISP W1024H768B32F85 or MENU -
```

## Remarks

- MENU is generally located beyond _SUB and _END.
- The icon for the tray icon menu is determined by the window icon defined by _Sub.If it is
  absent,the main icon of PECMD will be adopted.
- MenuItemTitle is referenced via the environment variable %MenuItemName%,including
  %MenuItemName.Enable%.
- Use "ENVI @MenuItemName=NewTitle" to set and modify MenuItemTitle.
- Use "ENVI @MenuItemName.Enable=Num" to set the serviceability of the editbox; 0:disabled;
  1:enabled.
- Use "ENVI @MenuItemName.ID=?[hmenu][;CtrlIDVarName][;MenuItemSN]" to query IDVarName etc.

