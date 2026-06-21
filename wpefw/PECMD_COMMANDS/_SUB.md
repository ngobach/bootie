# _SUB CLASS WINDOW FUNCTION

To define a class,subfunction(process)or window,supporting nested definition of a subclass.

## Syntax

```text
_SUB <SubprocName> [*]  //* indicates the function is of THIS type,so is its being called.
_SUB <WinName>,<WinShape>,[WinTitle],[[=]Event],[WinIcon],[WinType],[Mask] [,-ntab -nfocus
     -top-forcenomin -disminb -maxb -discloseb -nosysmenu -nocap -[dis]altmv -size -trap
     -na -scale[f|:DPI]]
_SUB SubprocName,*,,destructing commands //a function with destructing commands(comma can be
     contained)(automatically executed before return).
```

## Parameters

- **SubprocName**: string.Being followed by ",*" indicates there being lasting stacks,and ",#"
  indicates a hidden window.
- **WinName**: string,should be unique,not the same as those of other controls or environment
  variables.
- **WinTitle**: characters.
- **WinShape**: position and size of a window.The format is <L(Left)T(Top)W(Wide)H(High)>,L,T,W
  and H are respectively followed by corresponding numbers.Center-aligned if L and T absent.
- **Event**: the command (by WinCMD) to execute upon the closing of a window.=:not close
- **WinIcon**: the icon for the titlebar in a window and displayed on the taskbar.The format is
  <IconFileName#ID>.
- **WinType**: [-][#][[$]Num][:[TranspColor]],"-":no titlebar,"#":no frame.1-99:transparency.
  $ before Num indicates values 0-255;100(255) indicates a hidden window.TranspColor refers
  to the color to be transparent,background color by default.
- **Mask**: [%FFFFFF%][*][<[W:H]>]bmpname indicates special-shaped window mask,generally black or
  white BMP(RLE compressed format permitted),GIF,JPG.[%FFFFFF%] indicates transparency,0
  (Black)by default.*[*] indicates setting as background [only]. ~:scale,#ID indicates
  internal BMP, ID refers to resource ID,like #100.When using transparent color, the font
  Quality is set to 3.
- **Status**: -top:top -forcenomin:not to be minimized. -disminb:to disable minimize button. -maxb:
  to enable maximize button. -discloseb:to disable close button; -nosysmenu:no system menu
  -nocap:no titlebar.-nfocus:no focus. -disaltmv:to disable pressing ALT to move it with
  mouse. -size:size ajustable. -trap:to close the button without exit. -ntab:no keyboard
  focus. -csize:custom size  -na:not active -scale:scale for win8  -scalef: surpport NT5
  -scale:DPI scale by user DPI  -nxp:NoXPStyle

## Examples

```text
_SUB DoLoop or _SUB Windows1,W360H440,WinCMD Function Demo,EXEC $USAGE.TXT,%IconFile%#19,20
_SUB F1,*,,IFEX #[ %&h%<>0xFFFFFFFF && %&h%>0 ],CALL $Kernel32.dll,CloseHandle,#%&h%
```

## Remarks

- The name of a subfunction or window cannot begin with "$",and there should be a space
  between the command keyword _SUB and the name of the subprocess or window.
- A name beginning with & indicates a local one.This helps to avoid conflicts in a thread,
  which can also be achieved by entering the local mode of the thread.
- If many _SUBs exist,names defined by them cannot be the same,approximated or set as
  environment variables.
- This command should be paired with _END,neither of them can be used in command line.
  Refer to END,CALL for more
- To set the title of a window,use "ENVI @WinName=WinTitle".                   Refer to ENVI
- After transparency(not 0) is set in "WinType",the control IMAG cannot display GIF.
- The subprocess has LAMBDA itself,so it is able to access PE variables of its caller.
- The subfunction names of different classes or subclasses can be the same.Member functions
  of a parent class are accessible to its successor.At least one space should be indented in
  nested definition.
- Connect the name of another class with the class field mark :: to access a subfunction of
  it.Beginning with :: means starting from the class of top level,multi-level is permitted.
- With its own stacks,the command automatically destruct PE variables,lock,controls and HKEY
  when it exits.
- " ." is appointed as the mark of data memembers of a class,like ENVI aa.bb.cc.d=1.
- _SUB becomes a shortcut to add user-defined commands after it is extended.It can be used
  as an internal command (CALL is not needed).
- A common function can also be called by the command of CALL as a window object,generating
  many objects(fibre).
- Use "ENVI @WinName or CtrlName.Enable=[#]Value" to set its serviceability,0:disabled,
  1:enabled(not to respond).#:used in subthread.
- Use "ENVI @WinName or CtrlName.Visible=[*]Value" to set its visibility,0:invisible,
  1:visible,* indicates the second plan.>=1,successively as follows:
  SW_SHOW,SW_SHOWNORMAL,SW_MAXIMIZE,SW_MINIMIZE,SW_RESTORE,SW_SHOWDEFAULT,SW_SHOWMAXIMIZED,
  SW_SHOWMINIMIZED,SW_SHOWMINNOACTIVE,SW_SHOWNA,SW_SHOWNOACTIVATE,SW_FORCEMINIMIZE.
  (Refer to supplements.doc)
- A window without titlebar can be dragged by press ALT and the left button of the mouse.
- "ENVI @Win.Paint=FuncName":to set canvas callback function with params HDC Width Height.
- "ENVI @WinOrCtrlName.InvalidateRect=<L:T:R:B>|#WID|@SubName[~ExtVal]:[EraseBkground]"refresh
- "ENVI @WinOrCtrlName.Cursor=CursorID[,][DLL File]". //to set cursor in a window.If the
  comma is absent,it refers to the system cursor.
- "ENVI @WinOrCtrlName.Font=[FontSize][:[FontName][FontAdorn]]". //to set font,refer to EDIT
- "ENVI @WinOrCtrlName.bkcolor=[BackColorVal]".     //to set backcolor and to clear if null.
- "ENVI @WinOrCtrlName.trans=0/1".         //0x1:to set the transparency mode of background,
  0x2:totally transparent.
- "ENVI @WinOrCtrlName.style=[@*]CancelStyle[:AddStyle]".ENVI @@style=%WID%:[@*]v1:v2
  //@:direct,*:extended
- "ENVI @Win.HitTest=Height[:Width:x:y]".  //to set the sensitive range of a movable window.
  If height<=0,cancelled.
- "ENVI @WinOrCtrlName.cmd[?]=Name|CMD"  //to set/query the response command dynamically.
  query need  "ENVI^ QueryCmd=1"
- "ENVI @WinOrCtrlName.nxp="  //disable XPStyle
  ◙Class and instance:One class can generate many instances simutaneously,each of which has
  its own name.In a window instance,THIS refers to the current real window instance.
- ENVI @@Enable=WinID:[#]Num",0:disabled(not to respond); 1:enabled. #:used in subprocess.
- ENVI @@Visible=WinID:[*]Num":to set its visibility,0:invisible,1:visible.*:the second
  plan,the same as above.
- ENVI @@Enable=?WinID:VarName":to return the status of serviceability.
- ENVI @@Visible=?WinID:VarName":to return the status of visibility.
- ENVI @WinCtrlName.Enable=?VarName":to return the status of serviceability.
- ENVI @WinCtrlName.Visible=?VarName":to return the status of visibility.
- ENVI @@IsWindow=?WinID:VarName":to return whether it is WinID or not.

