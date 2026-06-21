# TIME

To create a timer in the window defined by _SUB.

## Syntax

```text
TIME [-t:[+]Times] [*] <TimerName>,<TimerCyc>,[TimerEvent]
```

## Parameters

  * before TimerName indicates being automatically recycled before quitting the block or
  function.
- **TimerName**: string,should be unique and not the same as those of other controls or
  environment variables.
- **TimerCyc**: number,ms as the unit,>0 indicates the timer starts working immediately and 0
  indicates the timer stops.
- **TimerEvent**: the command to execute when the timer is triggered,a valid one supported by
  WinCMD.

## Examples

```text
TIME Timer1,10000,FILE %TEMP%\*.*
```

## Remarks

- TIME can be located beyond _SUB and _END to execute some commands.
- The environment variable %TimerName% refers to the working status of the timer.0:stopped,
  not 0:started.
- Use "ENVI @TimerName=0" to stop the timer,and use "ENVI @TimerName=TimerCyc[<;|,>Times]"
  to restart.
- Use "ENVI @CtrlName.*del="(or "ENVI @TimerName=-del")to delete.After that a new one can be
  redefined.

