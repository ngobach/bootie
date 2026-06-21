# LOGS

To create a log file to record the execution outcome of every command so that users can
verify the validity of the config file.

## Syntax

```text
LOGS [--thread[@+] --lock:CheckNum --RAW=0/1 --ON=0/1 --p[-] --2[-] --t=0/1 --ln=1/0
     [--rt=1/0 --a --abak --tmp --vol[+] --part --rt[?]] [[*?FilePath][,Num]]
LOGS *   //to flush to the disk immediately and then release memory without closing the log
```

## Parameters

  ☼FilePath:refers to the name of the log file including its path.
  --thread for cur thread only. @:Common  +:Both  None for Close:Close Both
  --rt or * indicates realtime mode and flushing to the disk immediately.
  --rt=1/0:to set the realtime mode.
  --p:to print before execution; --2:to print before and after execution;
  The suffix - means cancel,--t= indicates printing the time thread,--a:to append to the LOG
  --ln:the line number;
  --np[-] indicates no logging according to the automatic judgement before execution;
  --ON= indicates the local switch to open or close the logs;
  --RAW= indicates the local switch to export raw text without interpreting variables in it.
  --rt? indicates the restoring mode(only to close the log file if "--rt?" is not followed
  by FilePath).
  --abak restore this LOGS on child LOGS out of range.
  --tmp retore old LOGS on this LOGS out of range. --lock:CheckNum:for operate
  --part &__LOGS is \\?\\Harddisk#Partition#  --vol &__LOGS is volume   --vol+BOTH
  LOGS --ln=0/1 LOGS --np[-] can be written as: ENVI^ logs_ln=0/1 ENVI^ logs_np=0/1
  ☼Num:unlimited actually.More records in the log file demand larger disk space.
  The command led by @ indicating not writing records into the log file.
  *?:equal to --rt --rt?

## Examples

```text
LOGS X:\PECMD.LOG or LOGS * CONOUT$ //to debug the terminal; LOGS --a --rt --rt? %&OldLOGS%
//to restore the LOG.
```

## Remarks

- The command of "LOGS" without any param should be called once more to close the log file
  before accomplishing the config file.It is automatically executed before the program exits
- This command is not applicabile to the command line but to config file.
- It is not suggested using a log file in some publicly-issued WinPE.It is only intended for
  debugging. Restricted in message processing.
- The config file lies in the directory containing the caller by default,as
  X:\windows\system32\pecmd.ini indicates X:\windows\system32\ and X:\OrderUSB.WCS indicates
  X:\; It is also permitted to directly specify FilePath like X:\pecmd_debug.log(X:R&W).

