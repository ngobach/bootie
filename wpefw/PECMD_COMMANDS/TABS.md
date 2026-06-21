# TABS

property table.

## Syntax

```text
TABS   [*] Name,<Shape>,PropPageSet,[Status]
```

## Parameters

  * before Name indicates being automatically recycled before quitting the block or
  function.
- **Name**: string,should be unique and not the same as those of other controls or
  environment variables.
- **Shape**: the position and size of the button.The format is <L(left)T(top)W(wide)H(high)>.L,T,
  W and H are respectively followed by corresponding numbers.
- **PropPageSet**: Page1ClsName[:Page1Name][:Page1Title][:Page1Tip]; Page2ClsName[:Page2Name]
  [:Page2Titel]:[Page2Tip]; ...
  ClsName is the window defined by _SUB.Variables are supported.PageName is the practical
  name for operation,which is the ClsName by default.
  A proper number of "-" being added before PageName of the subinterface indicates directly
  promoting the PE variables of the subinterface by corresponding levels.
- **Status**: number,minus:disabled button,0x10:invisible,4:multiline,0x20:wide mode,0x40:with
  frame,0x80:vertical setting of types.
  THICKFRAME=0x40000  DLGFRAME=0x400000 BORDER=0x800000

## Examples

```text
TABS TABS1,L32T108W300H54,Page1:P1:title1:tip1;Page2:P2:title1:tip1
```

## Remarks

- TABS is generally located between _SUB and _END.                   Refer to _SUB and CALL.
- To set the text of the property page:"ENVI @Name.TilteSN=Txt",SN starts from 1.
- Use "ENVI @Name.Enable=Num" to set its serviceability; 0:disabled,not 0:enabled
- Use "ENVI @Name.Visible=Num" to set its visibility; 0:invisible,not 0:visible.
- Use "ENVI @Name.Select=SN" to set the selected page; >=1.  %&Name.Select%  %&Name.TilteSN%
- To set the tips on the property page:"ENVI @Name.TipSN=Txt". %&Name.TipSN%
- In window instance,this refers to the current real window instance.- refers to the window
  instance of parent level.Instances of different levels are separated by : like:
  ENVI @this:P1:ITEM1.Enable=1

