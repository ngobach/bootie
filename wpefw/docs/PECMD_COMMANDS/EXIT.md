# EXIT

To exit the subprocess called by the current command of CALL,the process of config file
called by LOAD or the loop command called by LOOP.

## Syntax

```text
EXIT [ScriptType]
EXIT= [ErrCode or PE_VarName or error] //to set ErrCode,0 by default,PE_VarName is its
address,error:to return the error code.
```

## Parameters

- **ScriptType**: indicated as [Num or CMD].If absent,it is automatically judged(_SUB,FILE,FORX,
  LOOP,BLOCK).
  ♦CMD:not case sensitive.FILE,WIN,_SUB,FORX or BREAK,LOOP or BREAK,- or BLOCK and CONTINUE
  respectively refer to the current file,current  window,current function,current FORX,
  current LOOP,jumping to the end of current block(before }) and jumping to the next round
  of LOOP/FORX. EXIT ToWin indicates terminating the execution of the function and return to
  the control of window.[suggested]
  ♦Num:taken from "1~4",referring to the current process,current file,current FORX,and
  current LOOP.[not suggested]

## Examples

```text
IFEX $%Val%=10,EXIT Loop or FIND $%Cancel%=YES,EXIT File!EXIT _Sub or EXIT 1
```

## Remarks

- Example 1 shows that when Val is 10,the program exits the current loop; The error code is
  thread-independant.

