# SEND

To simulate keys/mouse.--ext:EXTENDEDKEY --s:SILENT  //support: ;  , :  /  ' '

## Syntax

```text
SEND [-gui[-m][-nfocus][-right|-left|-top]] [--ext] [--s] <KeyCode1[_|^]>[;KeyCode2]...
SEND -m  flag;dx;dy[;dat;[extdat]]                               //send mouse event
```

## Parameters

- **Virtual key code**: like VK_NUMLOCK.Refer to the list of vitual keys behind and related
  programming materials for more.If KeyCode ends with "_",it only simulates the key being
  pressed down and "^" only simulates the key being up.Otherwise,it simulates both.
- **KeyCode**: Simple keys("a"~"z" or "0"~"9") can be indicated as letters directly; Other keys
  should be indicated with "#" followed by the virtual key code.
  If KeyCode ends with "_",it only simulates the key being pressed down and "^" only
  simulates the key being up.Otherwise,it simulates both.
  The code without the leading character # and whose length is >=2 is supported,as well as
  the one beginning with 0x. The name strings of VK_ are also supported.
- **flag**: 0x8000:absolute position. 1:Move.2/4:Press/Release left button. 8/0x10:Press/Release
  right button. 0x20/0x40:Press/Release middle button. 0x80/0x100:Press/Release X button.
  0x800:wheel button rolled. 0x4000:map to entrire virtual desktop. 0x100000:TouchScreen
  dx/dy:absolute/relative pos. dat:0 or 120 when wheel rolled and clicked. extdat:0 for now

## Examples

```text
SEND #0x12_,#0x09_,#0x09^,#0x12^  or SEND   VK_NUMLOCK
SEND 0x12_,0x09_,0x09^,0x12^         //old format
```

## Remarks

  The example above is to simulate Alt+Tab.The KeyCode supports hex and decimal.

