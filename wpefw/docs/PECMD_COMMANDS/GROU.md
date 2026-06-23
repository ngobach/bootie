# GROU

To create a combined panel in the window defined by _SUB and illustrate the functions of
the controls in it.

## Syntax

```text
GROU [-right] [-center] [*] <Name>,<Shape>,Title,[Status][,FrontColor#BackColor][,Font]
```

## Parameters

  * before <ComPanelName> indicates being automatically recycled when quitting the block or
  function.
- **ComPanelName**: string,should be unique and not the same as those of other controls or
  environment varialbes.
- **ComPanelShape**: the position and size of the combined panel.The format is <L(left)T(top)
  W(wide)H(high)>.L,T,W and H are respectively followed by corresponding numbers.
- **ComPanelTitle**: string,to describe the functions of the combined panel.
- **ComPanelStatus**: ±16:invisible   Font:see EDIT

## Examples

```text
GROU Group1,L8T4W336H400,to register WimShExt.DLL
☼This command is generally located between _SUB and _END.           Refer to _SUB and CALL.
```

## Remarks

- To set the title of the combined panel,use "ENVI @ComPanelName=ComPanelTitle".Refer to
  ENVI.
- Use "ENVI @ComPanelName.Visible=Num" to set its visibility.0:invisible,not 0:visible

