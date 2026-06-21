# SPIN

To create a button with up and down keys in the window defined by _SUB.

## Syntax

```text
SPIN [-right] [-left] [*] <ButtonName>,<Shape>[,ValInfo][,CMDParamVarName][,CMD][,Status]
```

## Parameters

  * before <ButtonName> indicates being automatically recycled before quitting the block or
  function.
- **ButtonName**: string,should be unique and not the same as those of other controls or
  environment variables.
- **Shape**: the position and size of the button.The format is <L(left)T(top)W(wide)H(high)>,
  L,T,W and H are respectively followed by corresponding numbers.
- **ValInfo**: [PartnerEDITVarName][:StartVal][:EndVal][:InitVal].Default:0:100:0.EDITVarName is
  prior to autobudy.
- **Event**: the command to execute when it is clicked,a valid one supported by PECMD.
- **CMDParamVarName**: [NewValVarName][:ButtonVarName][:OldValVarName].The ButtonVarName returns
  0/1,corresponding to the down button/up button.
- **Status**: number,minus:disabled,0x10:invisible,0x20:wrap,0x40:horizontal,0x80:to autobudy the
  previous control.

## Examples

```text
SPIN SPIN2,L32T108W300H54,edit1:-10:10,&&pos:&&id:&&oldv, CALL OnSPIN1 %&pos% %&id%,0x40
```

## Remarks

- SPIN is generally located between _SUB and _END.             Refer to _SUB and CALL
- To set the text on SPIN,use "ENVI @SPINName=ButtonTxt".   Refer to ENVI
- Use "ENVI @SPINName.Enable=[#]Num" to set its serviceability.0:disabled; 1:enabled(not to
  respond).#:used in the subprocess.
- Use "ENVI @SPINName.Visible=Num" to set its visibility.0:invisible; not 0:visible.
- Use "ENVI @SPINNaem.VAL=[CurVal][:StartVal][:EndVal]" to set the ValInfo.
- Use "ENVI @SPINName.VAL=?[CurValVarName][:StartValVarName][:EndValVarName]" to query the
  ValInfo.

