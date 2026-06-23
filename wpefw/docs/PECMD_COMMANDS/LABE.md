# LABE

To create a static text label in the window defined by _SUB.

## Syntax

```text
LABE[-|+.*>] [-right -center -left -trans -nf -w -vcenter -ncmd -3D] [-mod] [*]
    <LabelName>,<LabelShape>,[LabelTxt],[*][CMD],[ColorSet],[FontSize:[FontName]]
```

## Parameters

- **- and | indicate horizontal and vertical scroll bar(only to be viewed,not changeable,or**
  use EDIT instead),+ indictates being with frame; . indicates not converting "\n" here;
  > means ENVI @ convert \n; * indicates preinterpreting the event(variable).Notice that the
  params above should follow the command of LABE closely without any space,regardless of
  sequence.[-right]text right aligned,[-center]text center aligned,[-left]text left aligned,
  [-trans]:transparent label,-3D:3D outline.-mod:outline. -nf:not flash. -w:Auto wrap
  * before <LabelName> indicates being automatically recycled before quitting the block or
  function.-vcenter is only applicable to a single-lined text.-ncmd:no features of a command
- **LabelName**: string,should be unique,not the same as those of other controls or environment
  variables.
- **LabelShape**: the position and size of the label.The format is <L(left)T(top)W(wide)H(high)>,
  L,T,W and H are respectively followed by corresponding numbers.
- **LabelTxt**: string,the text to be displayed in the label,supportive of display in multiline,
  "\n" as the line break.
- **CMD**: valid command of WinCMD,like EXEC $%CurDir%\a.TXT or EXEC $http://mdyblog.blog.163.com
  * indicates not displaying a hand-shaped cursor.
- **ColorSet**: number,consisting of such 4 groups of colors as the normal text color,normal
  background color,active text color and active background color respectively,separated by
  "#",like "Num1#Num2#Num3#Num4",hex number supported,like 0x00FFFF#0xFF0000#0x0000FF#0000.
  Any of them absent,a system default value will be adopted.If no # follows a certain number
  in the second active color group,it will be the same as the corresponding color in the
  former normal group; If there is no or only one #,the two groups of color will be the same
- **FontSize**: number,hex number supported,like 0x0A.If absent,the system default font size will
  be adopted.16:invisible.

## Examples

```text
LABE Label1,L20T280W128H48,Please click "Open" to view the files.
LABE Label2,L20T24W80H20,Help Instruction,EXEC %CurDir%\ReadME.EXE,0xFF0000,12
LABE Label3,L20T48W80H20,DSystem,EXEC $http://mdyblog.blog.163.com,0x000000##0x0000FF,14
```

## Remarks

- LABE is generally located between _SUB and _END.                  Refer to _SUB and CALL.
- If "BkColor" or "CMD" is set,"LabelTxt" will be displayed center-aligned and the center
  aligned text cannot be displayed in new lines.
- To set the text on the label,use "ENVI @LabelName=LabelTxt".              Refer to ENVI.
- If "CMD" refers to executing websites by the command of EXEC,a string of website "//" can
  be contained.
  If "CMD" is present,the mouse will become a hand-shaped one when it moves on to it.
- Neither "LabelTxt" nor "CMD" is allowed to contain the character ",".If they must,a
  variable can be employed to store the character for its referencing(%DouHao%).
  "LabelTxt" can be set in the next command of ENVI,or set by ENVI in a timer,which is
  triggered once after the window is started.The "CMD" can be fulfilled by the command of
  CALL and executed in the mode of preinterpretation(%MYCMD%).
- The system default font size and color may vary in different systems as it has something
  to do with the system settings.
- Use "ENVI @LabelName.Visible=Num" to set its visibility; 0:invisible; not 0:visible.
- As for the message,there is no default one.The message will be activated if there is a
  command.If there is no command yet a message is needed,it can be placed in the command
  field with *.Once the message is activated,the control shielded by it will be inoperable.
- Use "ENVI @CtrlName.*del=" to delete.

