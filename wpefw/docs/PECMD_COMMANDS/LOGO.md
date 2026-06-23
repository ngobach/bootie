# LOGO

To set or close the login screen.-:quick exit without fading out.-top:to be on the top.
-enable:to press ESC to exit.-wait:to wait for the end of the animation.

## Syntax

```text
LOGO [-] [-enable] [-wait] [-trans:Transparency] [FilePath][,Color]
LOGO [-] [-enable] [-top] [-trans:N] --[,TranspColor] //to prepare a vancus for the command
of TEXT,background color by default.
```

## Parameters

  ☼FilePath is the name of a picture(path included) supportive of the format BMP/JPG/PNG/GIF
  and so on(need the support of GDI+).
  ☼Color indicates the background color,hex number supported.If both the LOGO picture and
  background color are set,the display priority will be given to the LOGO picture.
  ☼If the LOGO picture is invalid,the background color will be enabled; If the background
  color is invalid,an internally defined background color in the registry will serve as the
  alternate.
  ☼If there are no params,the startup screen will be closed(fading out).Transparency:0-255
  //The transparent color is not applicable to a text.

## Examples

```text
LOGO %SystemRoot%\XCMD.JPG
```

## Remarks

- This command is executed in a non-blocking mode.The next command will be immediately
  executed after its execution is finished.Therefore,if it is used in a command line,use the
  command of WAIT to delay,or the program will exit right away without displaying the effect
  of execution.
- This command can be emloyed for many times.Changing different pictures in the process of
  startup is practicable if you want to do so.
- A call of the command "LOGO" without any param is required before quitting the config file
  to close the startup screen.
- The memory for the LOGO picture is required to be a bit larger when it is used.

