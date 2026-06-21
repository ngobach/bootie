# EDIT

To establish a single-line/multiline editbox.

## Syntax

```text
EDIT[-|+.*=] [-right] [-center] [-vcenter] [-rich] [-3D] [*]
   <EditBoxName>,<EditBoxShape>,[EditBoxContent],[EditBoxEvent],[EditBoxType],[Color],[Type]
```

## Parameters

- **- indicates horizontal scroll bar; | vertical scroll bar; + no frame; .not to convert \n,**
  convert otherwise; * to preinterpret the event(variable); = the content of the editbox is
  the file name.The params above should closely follow the command word "EDIT" without any
  null character,not sequence sensitive.
  The text is left-aligned by default,-right:right-aligned,-center:center-aligned,-rich:to
  to set colors,-3D:3D profile. -u2d:unix2dos
  * before <EditBoxName> indicates being automatically recycled before quitting the block or
  function.
- **EditBoxName**: string,should be unique and not the same as those of other controls or
  environment variables.
- **EditBoxShape**: position and size of the editbox.The format is:<L(left)T(top)W(wide)H(high)>,
  L,T,W and H are respectively followed by corresponding numbers.
- **EditBoxContent**: string,characters to input when initializing the editbox. \n or native
  newline.The input of a comma,as well as an event,can be achieved by transfering to a
  variable.
- **EditBoxEvent**: the command to execute when ENTER is pressed in the editbox or the editbox is
  double clicked.It should be a valid one supported by WinCMD.
- **EditBoxType**: number(accumulated),0 by default as a normal editbox,minus:disabled,1:password
  inputbox,8:read-only,4:multiline,0x10bit:invisible,0x20bit:editable and supportive of
  automatical newline,0x40/0x800bit:to jump to the end/EOL; 0x100:to receive a name of file
  dragged in; 0x200:to receive all names of files dragged in,multiline; 0x400:number,
  compatible:separate 2=disable,3=read-only.
- **Color**: number,including text color and background color.Format:"Num1#Num2",default color if
  absent.
- **Font**: The format is "font size[:Font]".
  Other adornment can be attached to the font name like [**BbUuIiSs#Weight#Width#CharSet#
  Quality#Orientation#Escapement#OutPrecision#ClipPrecision#PitchAndFamily].Also applicable
  to settings of other fonts.

## Examples

```text
EDIT Edit1,L32T244W240H24,C:\Windows,ENVI @Label1=%Edit1%,0x224
```

## Remarks

- This command is generally located between _SUB and _END.               Refer to _SUB,CALL.
- Use "ENVI @EditBoxName=TextContent" to set the content of the editbox.     Refer to ENVI.
- Use "ENVI @EditBoxName.Enable=[#]Val" to set its serviceablity.0:disabled; 1:enabled(not
  to respond).#:used in a subthread.
- Use "ENVI @EditBoxName.ReadOnly=Val" to set the status of read-only; 0:not read-only,not 0
  :read-only.
- Use "ENVI @EditBoxName.Visible=Val"to set its visiblity.0:invisible,not 0:visible.
- Use "ENVI @EditBoxName.LINE=Val" to make some line visible,but the line cannot be queried.
  Line ID starts from 1. 0 and 1 indicates jumping to the line ID 1 directly,and "-1" to
  the end.
  Being led by : indicates a relative position.A minus indicates the number of lines to
  scroll up.
- Use "ENVI @EditBoxName.Val=?VarName" to obtain the realtime content.
- Use"ENVI @EditBoxName.Color=[:FontSize[:Font:]BITUL;][Color[#BkColor]][;StartPos[;EndPos]]"
  to set colorful font.The position is calculated according to the content,starting from
  0.Position being absent indicates the currently-selected range.-1 indicates the end.
  The EndPos is -1 by default.A Chinese character and a line break occupy one position.
  B:bold,I:italic,U:underline,L:link,T:strickout.
  The position can be "LineID:[ColID]",LineID and ColID start from 1.
- Use "ENVI @CtrlName.*del=" to delete.

