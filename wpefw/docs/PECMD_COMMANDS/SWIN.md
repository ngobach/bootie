# SWIN

attribution list.
  * after SWIN indicates being automatically recycled before quitting the block or function.
■PicBoxName:the name of the picture box.

## Syntax

```text
SWIN   [*] [PicBoxName]:ClsName:[Name],<Shape>,[InternalPos][,Status] //picture in picture
SWIN   [*] -:ClsName:[Name],<Shape>,,[Status]
SWIN  -sub[:-#LxTyWwHh]] [ClsName]:[Name][,][CMDLineParam]       //external subwindow
                                                                 -:with title, #:with frame
```

## Parameters

- **ClsName**: the window defined by _SUB.An instance of window will be created according to it.
- **Name**: string,the name of the window instance should be unique,not the same as those of
  other controls or environment variables.The default is a class name.
  A proper number of "-" being added before PageName of the subinterface indicates directly
  promoting the PE variables of the subinterface by corresponding levels.
- **Shape**: the position and size.The format is:<L(left)T(top)W(wide)H(high)>.L,T,W and H are
  respectively followed by corresponding numbers.
- **InternalPos**: the position in the picture box.The format is <L(left)T(top)>.L and T are
  respectively followed by corresponding numbers,0 by default.
- **Status**: number,minus:disabled,0x10:invisible,0x40:with frame,0x80:horizontal scroll bar,
  0x100:vertical scroll bar.

## Examples

```text
SWIN -:Page1,L42T5W194H125 or  SWIN Swin1:Page1,L42T5W194H125,,0x1C0
```

## Remarks

- SWIN is generally located between _SUB and _END.                   Refer to _SUB and CALL.
- Use "ENVI @Name.Enable=Num" to set its serviceability.0:disabled,1:enabled.
- Use "ENVI @Name.Visible=Num" to set its visibility; 0:invisible,1:visible.
- In the instance of window,this refers to the current real instance of window.- refers to
  the window instance of parent level.Instances of different levels are separated by : like:
  ENVI @this:Page1:ITEM.Enable=1

