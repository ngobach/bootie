# ITEM

To create a button in the window defined by _SUB.-pcenter:picture center aligned; -round:
oval button; -nscope:no frame;  -def:ENTER applicable;  -na:not active -b:bind to parent

## Syntax

```text
ITEM [-na] [-b[:ParentHwnd]] [-right] [-left] [-def] [-font:FontSize:FontNameAdorn] [*]
     <ButtonName>,<ButtonShape>,[ButtonTitle],[ButtonEvent],[ButtonIcon],[Status]
```

## Parameters

  * before <ButtonName> indicates being automatically recycled before quitting the block or
  function.-nfocus:no focus.
- **ButtonName**: string,should be unique and not the same as those of other controls or
  environment variables.
- **ButtonShape**: position and size of the button.The format is <L(left)T(top)W(wide)H(high)>.
  L,T,W and H are respectively followed by corresponding numbers.
- **ButtonTitle**: characters on the button to describe the function of the button and the
  command to execute.
- **ButtonEvent**: the command to execute when the button is clicked.It should be a valid command
  supported by WinCMD.
- **ButtonIcon**: the icon displayed on the button with the format as <IconName#ID>,IconSize =
  ButtonH - 6.
- **ButtonStatus**: number,default 0:enabled,minus:disabled,16:invisible,4:in multiline.

## Examples

```text
ITEM Button3,L32T108W300H54,resource manager,EXEC explorer.exe,%SystemRoot%\explorer.exe
```

## Remarks

- This command is generally located between _SUB and _END.          Refer to _SUB and CALL.
- Use "ENVI @ButtonName=ButtonTxt" to set the text on the button.   Refer to ENVI.
- Use "ENVI @ButtonName.Enable=[#]Num" to set its serviceablilty,0:disabled; 1:enabled(not
  to respond).#:used in the subprocess.
- Use "ENVI @ButtonName.Visible=Num" to set its visibility; 0:invisible,not 0:visible.
- Use "ENVI @ButtonName.color=[Color]" to set the text color.
- Use "ENVI @CtrlName.*del=" to delete.

