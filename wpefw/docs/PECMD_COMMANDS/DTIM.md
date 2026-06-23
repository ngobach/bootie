# DTIM

To create a date/time indicator.
  * before <Name> indicates being automatically recycled before quitting the block or
  function.

## Syntax

```text
DTIM [-right] [*] <Name>,<Shape>,[InitVal],[Event],[Type]
```

## Parameters

- **Name**: string,should be unique and not the same as those of other controls or environment
  variables.
- **Shape**: position and size of the editbox.The format is:<L(left)T(top)W(wide)H(high)>,L,T,W
  and H are respectively followed by corresponding numbers.
- **InitVal**: string,characters to input when initializing the editbox.\n or the native newline.
  The input of a comma,as well as an event,can be achieved by transfering to a variable.
- **Event**: the command to execute when ENTER is pressed in the editbox or the editbox is double
  clicked.It should be a valid one supported by WinCMD.
- **Type**: number(accumulated),0 by default as normal editbox,minus:disabled,0x10bit:invisible,
  0x20bit:long date format,0x40bit:time format,0x80bit:short century date format,short date
  format by default,0x100bit:to ajust with up and down,0x200:with checkbox.

## Examples

```text
DTIM Dtim1,L32T244W240H24,2008; 5; 12,Mess,0x200
```

## Remarks

- This command is generally located between _SUB and _END.               Refer to _SUB,CALL.
- Use "ENVI @Name.Enable=[#]Val" to set its serviceablity.0:disabled; 1:enabled(not to
  respond).#:used in a subthread.
- Use "ENVI @Name.Visible=Val" to set its visiblity.0:invisible,not 0:visible.
- Use "ENVI @Name.VAL=Val1; Val2; Val3" to set year,month and day or hour,minute and second.
- Use "ENVI @Name.VAL=?Name1; Name2; Name3; Name4; Name5" to return the mark of year,month,
  day and week or hour,minute and second.mark:
  0:effective,1:to cancel checked,-1:failed.
- Use "ENVI @CtrlName.*del=" to delete.

