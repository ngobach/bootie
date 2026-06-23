# RADI

To create a radio in the window defined by _SUB.

## Syntax

```text
RADI [-right -center -scale[:[Sz][:Pic]]] [*] Name,Shape,Title,Event,[Status],[GroupID]
```

## Parameters

  * before <RadioName> indicates being automatically recycled before quitting the block or
  function.
- **RadioName**: string,should be unique and not the same as those of other controls or
  environment variables.
- **RadioShape**: the position and size of the radio.The format is <L(left)T(top)W(wide)H(high)>,
  L,T,W and H are respectively followed by corresponding numbers.
- **RaidoTitle**: characters on the radio to describe its functions or instructions of the radio.
- **RadioEvent**: command to execute when the radio is clicked,a valid one supported by WinCMD.
- **RadioStatus**: number,1 or -1:checked; 0,2 or -2:unchecked; <0:disabled; ±16:invisible.
- **RadioGroupID**: number,0 by default.Raidos can be divided into groups,in which a radio can be
  selected out of all without mutual influence among the groups.

## Examples

```text
RADI RadioButton1,L32T314W100H20,unload WimShExt.DLL,ENVI @Group1=%RadioButton1%,1,1
```

## Remarks

- RADI is generally located between _SUB and _END.                  Refer to _SUB and CALL.
- To set RadioTitle,use "ENVI @RadioName=RaidoTitle".                        Refer to ENVI.
- Use "ENVI @RadioName.Check=Num" to set the status of checking.0:unchecked; not 0:checked.
- Use "ENVI @RadioName.Enable=Num" to set its serviceability.0:disabled; not 0:enabled.
- Use "ENVI @RadioName.Visible=Num" to set its visibility; 0:invisible,not 0:visible.
- Use "ENVI @CtrlName.*del=" to delete.   -scale:Internal scaling.

