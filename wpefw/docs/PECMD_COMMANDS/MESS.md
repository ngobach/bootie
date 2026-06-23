# MESS

To display a [Yes/No] message window or a message window with a [Confirm] button.

## Syntax

```text
MESS[-*=.-svr-svr2-max-min-size-close-top-icon[-x:x][-y:y][*]Num][-sub:LxTy:ObjName]
    [TxtContent][@MsgWinTxtTitle][#MsgWinType[*TimeAutoClz(ms)][$DefaultOption]]
```

## Parameters

- **MsgWinTxtContent**: supportive of a multiline text.Use "\n" to break the text lines or adopt
  the primitive multiline text.Any character is permitted.Null characters behind MESS will
  be retained.
  "." or -raw after MESS indicates not converting "\n".Special characters can be transfered
  to a varialbe.-svr or -svr2:to display the window before login.
  +max:with maximize button, +min:with minimize button, +size:size ajustable, +close:without
  close button.+top:to be on the top,-x/y:position on the screen. +var:Text is VAR
  insvr:Display on User desktop in service. +txt static text  +cb to clipboard
- **MsgWinTxtTitle**: any single-lined text except @. Null characters behind MESS will be
  retained.Special characters can be transfered to a variable(%Mouse%).
- **MsgWinType**: Presently support "YN","OK",“YNC","OKC","RETRY" and "ABORT"."OK" by default.
- **If time for automatically closing is set as 0 or not set,the message window will not be**
  closed automatically.The default selection is indicated as Y or N,like $Y,$N,$C,$1($O),$O,
  $2($C),$3($A),$A,$4($R),$R,$5($I),$I,$6($Y),$7($N).A minus will not display the counting.
- **MESS-bin or "MESS*" indicates the concurrent call enabling the message window to be**
  simutaneously operated together with its parent window.The next command cannot be executed
  before it is closed.
- **MESS-bg or MESS indicates the backstage call.The subsequent commands will be continued but**
  some time-consuming operations will still be blocked.
- **MESS-tbin or "MESS~" indicates backstage call.The subsequent commands will be continued**
  and some time-consuming operations will not be blocked.It can be used for debugging.
- **"MESS=" indicates the blocked call.The whole message loop will be blocked**
  (applicable to soem special debugging).
- **"MESS" indicates the blocked call.The whole message loop will not be blocked.**
- **Number after +icon**: 0~7.No number,no icon.0:bang/1:warning,2:information/3:asterisk,
  4:problem,5:stop/6:error/7:waved stop.
  >=32 or led by * indicates being user-defined,corresponding to the number of IconGroup.
  Icons can be added on one's own.102 indicates a green checkmark.+icon**DLLEXE File#Number
  is also practicable.

## Examples

```text
MESS Insufficient physical memory,\n Set virtual memory? @Set virtual memory #YN *10000 $N
```

## Remarks

- User's selection to the message window will be stored in the variable %YESNO%,%&YESNO%.
  If [Y] is selected,its value will be YES,and NO for [N].If the window is closed overtime,
  the default selection will be returned.The command of FIND is applicable to judging the
  value of this environment variable.
  Other returned value includes OK,CANCEL,RETRY,IGNORE and ABORT.
- If the param of default selection is left out from the message window of "YN" type,the
  returned value after the window automatically closes will be NO.For the convenience of
  debugging,all params can be absent.

