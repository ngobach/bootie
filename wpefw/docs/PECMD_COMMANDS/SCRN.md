# SCRN

To return the current width and height of the screen.-win:as large as a maximized window.

## Syntax

```text
SCRN [-win]  [W-VarName],[H-VarName][,x-VarName][,y-VarName][,TaskBarPositionVarName]
SCRN -cap[gui]  [[:fmt:]File],[#WinID|<x:y:R:B>[;SrcFile]] //CaptureScreen fmt as:image/png
```

## Parameters

- **VarName**: to store the current width and height of the screen.

## Examples

```text
SCRN ScrW,ScrH   SCRN ScrW,ScrH  or  SCRN -cap  a.bmp,0   or   SCRN -cap  win.bmp,#1181336
```

## Remarks

- VarName cannot be the same as the existing variable names.If the command ends up with an
  error,the returned outcome for the two variables will be null or 0.
- Capture:Capture if CountPosParam>=1.Save last if no PosParam or WindID.Left if no FileName

