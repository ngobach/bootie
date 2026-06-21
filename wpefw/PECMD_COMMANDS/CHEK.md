# CHEK

To create a checkbox in the window.

## Syntax

```text
CHEK [-right -center -scale[:[Size][:PIC]]] [*] <Name>,<Shape>,[Title],[Event],[Status]
```

## Parameters

  * ahead of <ChkName> indicates being automatically recycled before quitting the block or
  function.
- **ChkName**: string,shoul be unique,not the same as those of other controls or environment
  variables.
- **ChkSize**: position and size of the checkbox.The format is <L(left)T(top)W(wide)H(high)>,
  L,T,W and H are respectively followed by corresponding numbers.
- **ChkTitle**: characters on the checkbox,describing its function or instructions.
- **ChkEvent**: the command to execute when it is clicked.The command should be supported by
  WinCMD.
- **ChkStatus**: number,1 or -1 indicates being checked,0,2 or -2 unchecked,smaller than 0 means
  disabled,±16 invisible.

## Examples

```text
CHEK Check1,L180T336W100H20,MountR.W.,,1
```

## Remarks

- This cammand is generally located between _SUB and _END.           Refer to _SUB and CALL.
- Environment variable:%ChkName% is the title of the checkbox.To set the title,use "ENVI @
  ChkName=ChkTitle"
- Use "ENVI @ChkName.Check=Val" to set the status of checkbox,0:unchecked,not 0:checked.
- Use "ENVI @ChkName.Enable=[#]Val" to set its serviceability,0:disabled,1:enabled(not to
  respond).#:used in a subthread.
- Use "ENVI @ChkName.Visible=Val" to set its visibility; 0:invisible,not 0:visible.
- Use "ENVI @CtrlName.*del=" to delete. -scale:Internal scaling.

