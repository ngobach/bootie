# RUNS

To set the startup item of Windows.

## Syntax

```text
RUNS <PrgmCMD>,<StartItemName>
```

## Parameters

- **PrgmCMD includes various kinds of executable commands like EXE,CMD,BAT and so on with**
  their params likely to be present.StartItemName is indicated in the form of characters.

## Examples

```text
RUNS PECMD.EXE EXEC !%CurDrv%\outpart\DRIVER\STARTDRIVER.CMD,Install drivers.
```

## Remarks

- The existing command of REGI can fulfill the same function.Only because the command line
  of REGI becomes too long does this separate command of RUNS come into being for this
  function.
  If there exists a single-byte English "," in PrgmCMD or StartItemName,it can be replaced
  with a Chinese double-byte one or transfered to a variable.
- This command is not applicable to the command line but to config file only.

