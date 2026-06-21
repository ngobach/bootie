# DISP

To set display params. *delay:to write to registry only.MultiDispMode:
0x81=1:to display,0x82:to copy,0x84:to extend,0x88=2:to display.

## Syntax

```text
DISP [-confirm] [-delay] [=DispID] [@PosMtrx] [S MultiDispMode] [P] [W HrzRsl H VerRsl]
     [B ColorDepth] [F RefreshRate] [T Wait ms] [O Ori]
DISP <?|?*>  [=DispID] VarName     //To query the current or all possible configs,multiline,
                                     separated by TAB in the line.   -<r|n>sort:sort
```

## Parameters

- **To specify the screen params respectively.All absent,set automatically.DispID starts from**
  0,P:to set as the main display. -size:only_size -reset -delay  -ori:Quey_Orient
  PosMtrx:X0:Y0:X1:Y1:X2:Y2:X3:Y3... coordinates of the base positions in all displays.
  -bright[?]:Val/Name  -guis:gui mode

## Examples

```text
DISP =1 S0x84 P W1024 H768 B32 F70 T5  or DISP  ? &CUR DISP ?* =0 &ALL or DISP =1 S0x84 P
```

## Remarks

  The first three groups of params can be used separately.Use DISP f75 to set the RefreshRate
  as 75. %&YesNo%=Yes:Set NO:Ret TIMEOUT:Ret on timeout

