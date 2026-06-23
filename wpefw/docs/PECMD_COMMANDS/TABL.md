# TABL

To make a table.
  * after TABL indicates being automatically recycled after quitting the block or function.
  -sps:The columns are separated by successive null characters.*:to select the default color
  as the one of selected cell,which is also applicable to the line selection if only one
  line is available for selection.

## Syntax

```text
TABL [-hfont:HeadFontSize[:FontNameAdorn]] [-font:FontSize[:FontNameAdorn][:HeadH:HeadColor:
      Grad:::FontH:FontW:LineH]] [-color:[BKColor][#TxtBKColor][#DefaultTxtColor][#SelCellCo
     lor]] [-sps] -ckszCheckBoxSz  [*] [Name],<Shape>[,FormatData][,Data][,Status]
```

## Parameters

- **Name**: string,the name of a window instance should be unique,not the same as those of other
  controls or environment variables.The default is the name of a class.
- **Shape**: the position and size of the table.The format is:<L(left)T(top)W(wide)H(high)>.L,T,W
  and H are respectively followed by corresponding numbers.
- **FormatData**: the head of the table.Columns are separated by TAB.For each column:
  "[#Color[#BKColor]][=|+]Width:Title".If absent,the number of the columns are determined by
  data.* before Width indicates left alinged(default),= right aligned,+ center aligned.
  The DefaultTxtColor is prior to others.It is the same with the added columns below.
- **Data**: the data to initialize the content of the table.Columns are separated by TAB and
  lines are separated by line breaks.
- **Status**: number,minus:disabled.0x10:invisible.0x40:with frame.0x80:without horizontal scroll
  bar.0x100:without vertical scroll bar.0x200:without grid lines.0x400:with check.
  0x800:to drag the headline to adjust the sequence of columns.0x2000:without headline.
  0x4000:to disable width ajustment.0x10000:only one line permitted to choose(highlight).
  0x40000:double click to select the cell.0x80000:to disable line selection.0x100000:to
  color the checked line.  0x200000/0x400000 1st/All column  may include pic.
  "*P:W{W}H{H}S{sW}V{sH}X{sX}Y{sY}|?File[>S{sW}V{sH}X{sX}Y{sY}|?|FileChk]%Nla%Text" ?[?]Ass-
  ociate[Replace] |Adv-Mode check/Sel {sW}{sH}{sX}{sY}:- indicates a multiple.0x800000/0x10
  00000: OnlyHori/vert line. Notice that selecting and checking are two separate solutions!
  1> click a line to color:0x10000
  2> double click the cell to select and color,disable line selection:
  0x40000+0x80000=0xC0000
  3> check but not color,disable line selection:0x80000+0x400=0x80400  //multiline check
  4> check and color,disable line selection:0x100000+0x80000+0x400=0x180400 //same above
  5> no check and color,disable line selection:0x100000+0x80000=0x180000 coordinating with
  Check=XX; 2                                         //multiline selection without check
  6> color freely:0x80000                               //ENVI @Name.color=Color

## Examples

```text
TABL TABL1,L42T5W194H125,50:C1%TAB%+50:C2%TAB%-50:C3,%MUI_LINE_DATA%
ENVI @TABL1.Sel=?.&Line,&Col
```

## Remarks

- TABL is generally located between _SUB and _END.                   Refer to _SUB and CALL.
- Use "ENVI @Name.Enable=Num" to set its serviceability; 0:disabled,1:enabled.
- Use "ENVI @Name.Visible=Num" to set its visibility; 0:invisible,1:visible.
- Use "ENVI @Name.Sel=LineID[; Val]" to select a line.LineID and ColumnID start from 1
  (the same below).Val:0/1,1 by default,0:to cancel the selection.2:selectd or
  unselected in turn.
- Use "ENVI @Name.Sel=?[*]VarName" to get the current LineID.<=0:not selected.*:to return
  all selected lines separated by space.
- Use "ENVI @Name.Sel=?.[LineVarName][; ColumnVarName]" to get the current position of the
  cell focused by mouse.<=0:invalid.
- Use "ENVI @Name.Sel=+Line; Column" to get the position of the currently selected cell.
  <=0:invalid.equal to double click.
- Use "ENVI @Name.Sel=?+[LineVarName][; ColumnVarName]" to get the position of the currently
  selected cell.<=0:invalid.
- Use "ENVI @Name.Val=?*;[NumLineVarName][; NumColumnVarName]" to get the numbers of lines
  and columns.
- Use "ENVI @Name.Val=?*.*; VarName" to get all data,ENTER as line breaks.
- Use "ENVI @Name.Val=?LineID[.ColumnID]; VarName" to query the content of a specified line
  /cell.Columns are separated by TAB(the same below).
- Use "ENVI @Name.Val=LineID.ColumnID; Val" to set the content of a specified cell.
- Use "ENVI @Name.Val=[>]LineID[*[*[*]]][#][.ColumnID][/LineH]; Val" to set the content of sp.
  line.The interval is specified by the switch of TABL.LineID 0 indicates adding a new
  line from the bottom and fill with the content.If there is ColumnID,fill this column
  with the content.* behind LineID indicates multiline is permitted.,**[*],add line[cel]
  on need.#:to eliminate the redundant at the end,and  "." can be absent.>:Insert
- Use "ENVI @Name.Val=-[*]LineID[#NumLine]" to delete this line or several lines.
  *:to delete to the end.
- Use "ENVI @Name.Val=.-ColumnID" to delete this column.
- Use "ENVI @Name.Val=+;[#Color][#BKColor][=|+]ColumnW:ColumnTitle" to add a column.
- Use "ENVI @Name.Check=?[*]VarName" to get LineID currently checked.Null means no checking.
  *:to return all lines checked,separated by space.
- Use "ENVI @Name.Check=?LineID;VarName" to query if the line is checked.
- Use "ENVI @Name.Check=LineID;Check" to set the status of checking.0:unchecked,1:checked
  2:checked or unchecked in turn.
- Use "ENVI @Name.Enable=?[*]VarName" to get LineID currently disbled. *:for all.
- Use "ENVI @Name.Enable=?LineID;VarName" to query if the line is enabled.
- Use "ENVI @Name.Enable=LineID;vEn[-]" to enable a line. 0:Disable,1:Enable，2:in_turn -:ns
- Use "ENVI @Name.Color=[LineID].[ColumnID];[Color][;BKColor]" to set the cell/line/col color.
  =?[LineID].[ColumnID];VarName;VarName; to query,return null if there is
  no color.The color of cell is prior to the one of line and the one of line prior to
  the one of column.-1 means eliminating the setting of color.
- Use "ENVI @Name.Color=ColumnID;[Color][;BKColor][;FontSize;FontName]" to set column_color
  =?ColumnID;[VarName][;VarName] to query,return null if no color.
- Use "ENVI @Name.Color=*LineID;[Color][;BKColor][/LineH][;FontSize;FontName]" to set LColor
  =?*LineID,ColorVarName,to query,return null if there is no color.
  If FontSize is -,it means eliminating the font.
- Use "ENVI @Name.UPos=?[@#][LnID].[ColID];L;T;R;B"to query pos. ?*LnID; ?ColID; @:ICO #LABEL
- Use "ENVI @Name.Percent=[*Line]|[Line.Col];[Percent][C|R|L|F|K][:BkColor][:PrgColor]
  [:TxtColor][:TXT]"   K:Skip CheckBox
- Expand "Val=" to  "Val=*:[l{LineSymbol}][c{ColumnSymbol}]:"  if needed.

