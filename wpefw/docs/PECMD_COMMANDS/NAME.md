# NAME    FNAM

To return the basic name of a file(without directory or extension); -ext:to retain extension

## Syntax

```text
NAME [-ext] <VarName><=><FileName>
FNAM [-ext] <VarName><=><FileName>
```

## Parameters

- **VarName**: to store the main file name;
- **FileName**: a valid file name.

## Examples

```text
NAME  fNam=C:\Windows\System32\calc.exe or NAME aExt=X:\Path1\Path2\FileName
```

## Remarks

- The outcome of example 1 is "calc",and example 2 is FileName;
- The returned value of FDIR,FDRV and FEXT can be checked or compared by the command of FIND

