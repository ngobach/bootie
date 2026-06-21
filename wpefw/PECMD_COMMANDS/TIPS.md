# TIPS

To display a bubble tip box in a specified position on the screen or in the tray.If all
params absent,invalid tray icons will be cleared.TIPS - indicates clearing the bubble tip
box.TIPS * indicates clearing the tray icons and bubble tip box.The suffix * indicates a
private control of TRAY in a window.If the tray is updated,the TrayIcon can be filled with

## Syntax

```text
TIPS[.*] [TipboxTitle],<TipboxContent>,[LifeMs],[PatrnID],[<@[A]TipboxPos>|[TrayIco]],[#WID]
TIPS#[.*][TipboxTitle],<TipboxContent>,#[LifeMs],[PatrnID],[<@[A]TipboxPos>|[TrayIcon]]
TIPS -dummy [?]Setting4Emu  //Set or clear Setting4Emu. Needed only once. ? only for win10
```

## Parameters

- **TipboxTitle**: string,the text displayed on the title of the tip box.Its length cannot exceed
  64 characters.The part of excess will be invalid.
- **TipboxContent**: string,the content of the text displayed on the tip box.Its length cannot
  exceed 256 characters,"\n" as the line break.
  "." indicates not converting \n. # indicates a necessary dilimiter,and the TipboxContent
  can be any character at this time.
- **LifeSpan**: number,the time for displaying the tip box(ms).If absent,10 seconds by default
  and the longest time for displaying is determined by the system.
- **PattnID**: number,0 by default(no icon),1(an information icon),2(a warning icon),3(an error
  icon),4 or above(tray icon).
- **TipboxPos**: with the necessary "@",it indicates displaying the tip box in a specified
  position on the screen."A" indicates a tip box in the shape of arrow and if "A" is absent,
  it indicates a square one.The position is indicated as LxxTyy(xx,yy are numbers) and this
  param cannot be used together with "TrayIcon".
- **TrayIcon**: The format is "[FileName][#Num]".If "FileName" is absent,use the icon specified
  in the program resource of PECMD.EXE.

## Examples

```text
TIPS Title,Content\nnewline permitted\nLine 3,5000,1,#1 or  TIPS -dummy  ?R*-20B*-20  or
TIPS Title,Content\nnewline permitted\nLine 3,5000,2,@aL600T400
```

## Remarks

- The tray bubble tip box ends as PECMD ends.
- The tip box on the screen ends as PECMD ends,so ensure that the life span of PECMD should
  be a bit longer than that of the tip box(use the command of WAIT to delay).

