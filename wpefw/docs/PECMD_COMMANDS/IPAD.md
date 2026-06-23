# IPAD

To create a control for IP address.
  * before <Name> indicates being automatically recycled before quitting block or function.

## Syntax

```text
IPAD  [*] <Name>,<Shape>,[InitVal],[Event],[Type]
```

## Parameters

- **Name**: string,should be unique and not the same as those of other controls or environment
  variables.
- **Shape**: the position and size of the editbox.The format is <L(left)T(top)W(wide)H(high)>,
  L,T,W and H are respectively followed by corresponding numbers.
- **InitVal**: string,AA.BB.CC.DD
- **Event**: the command to execute upon the notification of data change.
- **Type**: number(in accumulation),0 by default,a normal editbox.Minus:disabled,0x10:invisible.

## Examples

```text
IPAD IPAD1,L32T244W240H24,192.168.1.100,Mess      ENVI @Name.VAL=?.FullIP
```

## Remarks

- This command is generally located between _SUB and _END.          Refer to _SUB and CALL.
- "ENVI @Name.Enable=[#]Num",0:disabled; 1:enabled(not to respond).#:used in the subprocess.
- Use "ENVI @Name.Visible=Num" to set its invisibility; 0:invisible,not 0:visible.
- Use "ENVI @Name.VAL=Num1.Num2.Num3.Num4" to set IP.
- Use "ENVI @Name.VAL=?[.]Name1; Name2; Name3; Name4" to return IP."." indicates Name1(If
  the control name Name1 is absent,so can "." together with "Name1")returning full IP.
- Use "ENVI @Name.VAL=#Pos" to set the position of the cursor.1~4.
- Use "ENVI @Name.VAL=#Pos:StartRange:EndRange" to set the range of numbers.0~255.

