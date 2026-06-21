# KILL

To terminate a specified process by force.NUM refers to the exit code of the process.
-svr2 aims at MESS-svr2, -force:to kill the process by force.  KILL**:Forced synchronization
-explorer indicates that explorer.exe will not automatically restart. -gui:process manager
-tree pid_tree, -1 once

## Syntax

```text
KILL[**] [-force] [-exitcode=NUM] [-explorer]   <ProcName|*pid|\[WinTitle]|@[WinName]|@@WinID
                                                 |**tid|*&hpid|**&htid>
KILL -svr2
```

## Parameters

  ProcName,that is EXE FileName(or path), including *?.pid refers to process ID(PID).If
  WinTitle/WinName is absent,the current window is referred to.

## Examples

```text
KILL WinLogon or KILL \%ABC% or KILL -exitcode=0 *123 or KILL @ABC
```

## Remarks

  This command will terminate all the processes whose names match the front part of "ProcName"
  Therefore if the param is absent,all processes are likely to be killed.

