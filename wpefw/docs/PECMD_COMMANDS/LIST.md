# LIST

To create a listbox in the window defined by _SUB.
◙Notice:-h:H will limit the height of pulling-down◙
  * before <ListboxName> indicates being automatically recycled before quitting the block or
  function.

## Syntax

```text
LIST [-h] [*] <ListboxName>,<Shape>,<ListboxContent>,[ListboxEvent],[InitSelItem],[Status]
```

## Parameters

- **ListboxName**: string,should be unique,not the same as those of other controls or environment
  variables.
- **Shape**: the position and size of the listbox.The format is <L(left)T(top)W(wide)H(high)>,
  L,T,W and H are respectively followed by corresponding numbers.
- **ListboxContent**: the items to select in the listbox.Items are separated by "|".
- **ListboxEvent**: the command to execute when the selected item is shifted to another in the
  listbox.It should be a valid one supported by PECMD.
- **InitSelItem**: the selected item in the listbox when initializing its settings.
- **Status**: number,minus:disabled.0x4:editable listbox,0x200:simple listbox,0x10:invisible,
  0x100:with automatic vertical scroll bar,0x400:to sort automatically,0x800:capital,0x1000:
  lowercase.0x2000:not to trigger a command when editing.0x4000:to support dragging files in
  0x10000:not to triger a command when setting. 0x20004:ReadOnly editable listbox
  0x40000/0x80000:Left/Top align 0x100000:Specifie width -vcenter:RefHeight:Vertical center.
  Mouse Hover:WM_COMMAND:CBN_HOVER=0x7001,lp=id     0x7002,lp=ListHwnd

## Examples

```text
LIST List1,L200T360W128H232,EXPLORER|XPLORER2|CMD,ENVI @Label1=%List1%,EXPLORER
```

## Remarks

- LIST is generally located between _SUB and _END.                  Refer to _SUB and CALL.
- Use "ENVI @ListboxName=[+|-|:]ItemContent or ItemList" to modify the items in the listbox.
  "+":to add items to the listbox with the format "ENVI @ListboxName=+item to add or select"
  "-":to delete the specified item with the format "ENVI @ListboxName=-item to delete"
  ":":to select the specified item with the format "ENVI @ListboxName=:item specified"
  No leading character indicates initializing all items of the listbox with the format
  "ENVI @ListboxName=Item1|Item2|Item3...".Practically only "Item1" is effective and it is
  the one to add or select,so it is with =+.Therefore,the item content in the listbox cannot
  begin with such characters as "+","-" and ":".They can be transfered to a variable.
- ENVI @ListboxName.ADDSEL=item to add or select.
- ENVI @ListboxName.DEL=item to delete.
- ENVI @ListboxName.SEL=item to select.
- ENVI @ListboxName.VAL=to reset multiple items to add.  //separated by |; the original ones
  are cleared.
- ENVI @ListboxName.ADD1=multiple items to add.          //separated by |
- ENVI @ListboxName.ADD=multiple items to add.           //string in multiline
- ENVI @ListboxName.QUERY=LineNum; VarName     //If LineNum absent,all returned in multiline
- Use "ENVI @ListboxName.Enable=Num" to set its serviceability.0:disabled,not 0:enabled.
- The variable %ListboxName% is the selected item(string) in the listbox.
- The variable %ListboxName.isel% is the sequence number of the selected item(starts from 1)
- ENVI @ListboxName.isel=the sequence number of the selected item.
- Use "ENVI @ListboxName.Visible=Num" to set its visibility; 0:invisible; not 0:visible
- Use "ENVI @CtrlName.*del=" to delete.

