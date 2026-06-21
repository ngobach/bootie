# SLID

## Syntax

```text
SLID [-right] [-left] [*] <SlidName>,<SlidShape>[,ValInfo][,CMDParamName][,CMD][,Status]
To create a slider in the window defined by _SUB.
```

## Parameters

  * before <SlidName> indicates being automatically recycled before quitting the block or
  function.
- **SlidName**: string,should be unique and not the same as those of other controls or
  environment variables.
- **SlidShape**: the position and size of the slider.The format is <L(left)T(top)W(wide)H(high)>,
  L,T,W and Hare respectively followed by corresponding numbers.
- **ValInfo**: [StartVal][:EndVal][:InitVal][:PageSize].Default:0:100:0.
- **Event**: the command to execute when it is clicked,a valid one supported by PECMD.
- **Status**: number,minus:disabled.0x10:invisible.0x40:horizontal.0x80:no slider.

## Examples

```text
SLID SLID2,L32T108W300H54,30:100:50,CALL OnSPIN1 %SLID2%,0x40
```

## Remarks

- SLID is generally located between _SUB and _END.                   Refer to _SUB and CALL.
- To set the text on SPIN,use  "ENVI @SLIDName=ButtonTxt".                    Refer to ENVI.
- Use "ENVI @SLIDName.Enable=[#]Num" to set its serviceability,0:disabled; 1:enabled(not to
  respond).#:used in the subprocess.
- USe "ENVI @SLIDName.Visible=Num" to set its visibility; 0:invisible; not 0:visible.
- Use "ENVI @SLIDName.VAL=[CurVal][:InitVal][:EndVal][:PageSize]" to set the ValInfo.
- Use "ENVI @SPINName.VAL=?[CurValVarName][:InitValVarName][:EndValVarName][:PageSizeName]"
  to query ValInfo.

