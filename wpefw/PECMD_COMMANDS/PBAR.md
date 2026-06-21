# PBAR

To create a progress bar in the window defined by _SUB.

## Syntax

```text
PBAR [*] [-smooth] <PrgBarName>,<PrgBarShape>,[PrgBarPrg][,[#Color:]Txt]
```

## Parameters

  * before <PrgBarName> indicates being recycled before quitting the block or function.
- **PrgBarName**: string,should be unique and not the same as those of other controls or
  environment variables.
- **PrgBarShape**: the position and size of the progress bar.The format is:<L(left)T(top)W(wide)
  H(high)>.L,T,W and H are respectively followed by corresponding numbers.
- **PrgBarPrg**: float(1~100),to initialize the progressive percentage of the progress bar,0 by
  default.

## Examples

```text
PBAR Pbar1,L360T11W428H16,0
```

## Remarks

- PBAR is generally located between _SUB and _END.                  Refer to _SUB and CALL.
- To set the progress and text of the progress bar,use "ENVI @PrgBarName=Prg[;[#color:]Txt]"
  See ENVI; If the value is set as -1,the progress bar will be hidden and numbers >100 will
  be ignored.
- Use "ENVI @PrgBarName.Visible=Num" to set its visibility.0:invisible,not 0:visible.
- Use "ENVI @PrgBarName.color=color" to set the color of text.
- Use "ENVI @PrgBarName.percent=[-]smooth" to switch to the smooth mode.

