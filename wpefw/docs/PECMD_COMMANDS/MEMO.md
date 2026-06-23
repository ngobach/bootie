# MEMO

To create a multiline editbox in the window defined by _SUB.

## Syntax

```text
MEMO[-|+.] [-right] [-center] [-vcenter] [-rich] [*]
<EditboxName>,<EditboxShape>[,EditboxContent][,DestFileName][,EditboxType][,EditboxColor]
[,Font]
```

## Parameters

- **- indicates no horizontal scroll bar,| no vertical scroll bar,+ no frame,.indicates not**
  converting \n or esle converting.Notice that params above should closely follow the
  command without a space,regardless of the sequence.
  The text is left aligned by default,-right:right aligned,-center:center aligned,-rich:
  supportive of setting colors. -u2d unix2dos
  * before <EditboxName> indicates being automatically recycled after quitting the block or
  function.
- **EditboxName**: string,should be unique,not the same as those of other controls or environment
  variables.
- **EditboxShape**: position and size of the editbox.The format is:<L(left)T(top)W(wide)H(high)>,
  L,T,W and H are respectively followed by corresponding numbers.
- **EditboxContent**: the content to fill the editbox when it is initialized,multiline supported,
  \n or the native newline to break the line.A comma can be transfered to a variable.
- **DestFileName**: to specify the name of the text file,from which the content of the editbox is
  loaded.It will be ignored if the content is already specified.
- **EditboxType**: Num(in accumulation),absent or 0 by default,indicating an editable multiline
  editbox."-":disabled; 0x8bit:read only; 0x10bit:invisible; 0x20bit:editable and support
  automatic newline; 0x40bit:to jump to the end; 0x100:to accept a file name dragged in;
  0x200:to accept all file names dragged in,in multiline; 0x400:number.(for compatibility)
  1-15:read-only.
- **EditboxColor**: font,see EDIT.

## Examples

```text
MEMO Memo1,L304T268W280H88,,%CurDir%\Readme.TXT,0
```

## Remarks

- MEMO is generally located between _SUB and _END.                   Refer to _SUB and CALL.
- If [EditboxContent] is not blank,[DestFileName] will be ignored.If blank,the content
  specified by [DestFileName] will be loaded.For the convienience of reading in the notepad,
  the overlong "EditboxContent" can be transformed into a new envrionment variable of string
  by combining the nestedly assigned environment variable and a string cut out from the rest
  of  EditboxContent.
- Use "ENVI @EditboxName.Enable=[#]Num" to set its serviceability,0:disabled; 1:enabled (not
  to respond).#:used in the subprocess.
- Use "ENVI @EditboxName.ReadOnly=Num" to set the status of read-only.0:not read-only;
  not 0:read-only.
- ForeColor and BkColor support hex numbers in the format of C,like 0xFFFFFF.
- Use "ENVI @EditboxName.Visible=Num" to set its visibility; 0:invisible; not 0:visible.
- Use "ENVI @EditboxName.LINE=Num" to make a certain line visible.  Refer to EDIT for more.
- Use "ENVI @EditboxName.Color=..."to set the color and font.                     See EDIT.
- Use "ENVI @CtrlName.*del=" to delete.

## Model / Use Cases

ѺHow to use the modified content in the editbox as a new variable by assigning it.
  A:%EditboxName% like:
  ENVI$ &&NL=0d 0a
      EDIT| EDIT2,L36T25W187H49,EDIT2,,0x24
      MEMO MEMO1,L38T81W182H77,MEMO1,,
  MESS. EDIT2⌠%EDIT2%⌡   %&NL%  MEMO1⌠%MEMO1%⌡@#OK

