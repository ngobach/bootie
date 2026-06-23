# IMAG

To create a PicBox in the window defined by _SUB. -smooth:smooth

## Syntax

```text
IMAG [-gui|-size|-real|-sel|-bupdate] [*] <Name>,[Shape],[Res],[CMD],[Color],[LinWd]
IMAG <-numicong|-numicon|-numbmp|-num:Type> VarName,,Resource  // to return the numbers of
icong,icon,bmp etc.and MaxRsrcID.Icong indicates icon group.
```

## Parameters

  * before <PicBoxName> incicates being automatically recycled before qutting the block or
  function.-bupdate:to force to be in the mode of viewing picture files.
- **PicBoxName**: string,should be unique and not the same as those of other controls or
  environment variables.
- **PicShape**: position and size of the picture box.The format is <L(left)T(top)W(wide)H(high)>,
  L,T,W and H are respectively followed by corresponding numbers.
- **ImgResrc**: the image file(supported by Windows)to display in the specified picture box,like
  #Num|ResrcDir.
- **CMD**: valid commands of WinCMD,like EXEC !%CurDir%\XXX.CMD or EXEC %CurDir%\Setup.EXE.
- **FrameColor**: number,consisting of the normal color and active color respectively.Two numbers
  are separated by "#",like "Num1 #Num2",hex supported,like 0x00FFFF#0xFF0000.If absent,it
  is the system default color.
- **FrameLineW**: number,indicates the size in pixel of the picture box.If "CMD" is absent or
  invalid,the active color of the frame will be invalid.-16:invisible.

## Examples

```text
IMAG Image1,L8T380W140H70,%CurDir%\logo.gif,EXEC %CurDir%\Setup.EXE,#0xFF00FF,2
ENVI @Image1.update=32:32:100:50::;shell32.dll#52 //change the icon. or IMAG -gui -real -sel
```

## Remarks

- This command is generally located between _SUB and _END.           Refer to _SUB and CALL.
- As WinCMD.EXE is a script interpreter,it is not suggested loading picture files of large
  size,or they will be displayed very slowly.Supportive of  BMP,BMP compressed in RLE,JPG,
  GIF(pictures),AVI animation and so on.(Others depend on how many graphics drivers are
  installed in WINDOWS)
- "ImgResrc" supports displaying GIF animations in the format of standard GIF89a/87a.At this
  time,the settings for FrameColor and FrameLineW will be invalid.
- "ImgResrc" also supports resource icons in EXE or DLL with the format as <FileName#ID|Dir>
  Refer to ITEM.
- If "CMD" not absent,the mouse will become a hand-shaped one when it moves to the picture
  box. AVI animation is supported.
- Use "ENVI @CtrlName.Visible=Num" to set its visibility; 0:invisible,not 0:visible.
- Use "ENVI @CtrlName.update=w:h[:x:y:FramColor:FramLineW][;[*][?|][<X:Y:W;H>]ResFileName]"
  to update, incapable of changing the type. *:rebuild (change type) ?:show on exist
- Use "ENVI @CtrlName.*del=" to delete,applicable to other controls. |:advance Image
- Use "ENVI @CtrlName.stat=VarName" to check whether the control is valid.
- Use "ENVI @CtrlName.delay=ms to delay" to set the time to delay for the intervals in a GIF
  animation.
  Place <X:Y:W;H> just before ResFileName (INC.<>), for part of it

