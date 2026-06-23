# WAIT

1.to continue with the command after waiting for a specified Time1.
  2.* indicates waiting for Time1 for the process with the process ID as "pid" to finish.
    (* without a number behind indicates a parent process,** grandparent process and so on;
    * with a number behind indicates this process, ** as its parent process and so on).
    *- or *0 or absent indicates no process to wait for.Then the specified file and
    directory will be deleted respectively and another command of PECMD can be executed
    after the process ends. -sys indicates the system proxy(indirect) mode,and an absolute
    path should be contained in the file name at this time.=tid indicates thread ID.
    $Handle indicates handle.
    -thread:Wait for all child thread.

## Syntax

```text
WAIT [-sys[0]] [-pad] [-cont] [-ncd] [-freemem] [-][Time1(MS)][,VarName]
     [*[pid|=tid]] [Delay2(MS)] [-delms:ms] [-del FileDir1]  [-del ...] ... [-cmd cmd]
WAIT -scanall [VarName]    //to return the scan table of all 256 key values.
WAIT -sys[0] [switch/param] -cmd task_sys_proxy_command_args // async task mode.
WAIT -sys[0]cmd sys_proxy_command_args //direct command mode.  0:only on system level
♫-cont:not to leave out the keys between two calls.Generally,at this time,wait -0 first and
  abandon the outcome. -pad:for 0-9 of NUM PAD
♫-ncd:not to transfer the current directory while waiting for the process or executing the
  command.(multithread)
♫"-":to stop waiting on receiving any key within the specified period of Time1.Otherwise
  waiting till time is out. freemem:free mem;alonely maybe
```

## Parameters

- **a specified time,MS as the unit. pid can be hex numbers.**

## Examples

```text
WAIT 5000 or WAIT 0,PKey or EXEC "%MyName%" WAIT * -del  . or  WAIT 0 *123 check process 123
```

## Remarks

- Eg,wait for a period of time after executing the command of SHELL and then execute the
  command of LOGO to close the login screen or perform other operations.Besides,if a program
  is really needed to be run after login,a shortcut can be created in the start group or
  "HKCU\Software\Microsoft\Windows\CurrentVersion\Run".
  WAIT will not block the graphic interface.The -del mode is applied to clearing temporary
  files(folder),which is retried regularly until it succeeds.
- If Time1 is 0,it indicates there is a pause and the pause can be continued by pressing any
  key,thus the circulation goes on.WAIT 0/-N can be applied to checking the key pressed down
  by the user.'A'~'Z' or '0'~'9' will be returned directly,otherwise the key will be
  returned in the form of 0xNN.The variable name is PressKey by default.&&PressKey.Hex
  returns 0xNN or -1.
- If 0<Time1<1,Time1*100000 indicates the number of dispatched messages in queue.
- WAIT -sys0cmd  ENVI &::__PECMD.ONSHELL.LOG=D:\DEBUG.OnShell.LOG //SHELL watches over LOG

