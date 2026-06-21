# THREAD  THRD

To start a thread and run a command(group).Command line params with %n should not be used in
the command group.If needed,convert with ENVI.

## Syntax

```text
THREAD[&][*][+][$][#] ⌠-exp⌡ ⌠-wait[x][ -here]⌡ [-link] [--st:NumByteStack] CMDGroup
```

## Parameters

- **Any valid command or command group.&**: to force variables to be PE ones,which helps to
  achieve muitithread in the simplest way.&*+$ are not sequence sensitive.*/-link:to remain
  the relationship of parent and child including the window,and the parent process cannot be
  operated until the subprocess is completed(Thread safety has already been supported by
  PECMD).The default separate mode indicates no relationship with each other.+ indicates the
  abandoned thread,ie,the program will not wait for its exit all along.$/-exp:to interpret
  the command group behind once in advance.-wait:to wait for the end of the thread(x:not
  to block), -here:as child of courrent PE stack (Only in this way can we operate tmpPeVar)
  .  -tid:TidName.  -htid:HandTidName -waitp:Wait for the thread before the process ends.
  ☼After a thread is created,%&&__LastTID% is the unique ID for the last created thread,and
  %&__TID% is the ID for current thread.CALL @~ CALL @^ MESS-

## Examples

```text
THREAD  CALL Hello  a1 a2 a3
TEAM THREAD*  CALL @Menu01|  CALL @WIN1    //to open two windows simutaneously.
WAIT *syscmd THREAD#  MESS  bbbb           //#:to start a thread in proxy mode.The proxy
exits if the thread ends.
```

