# FEXT

To return the extension of the specified file(without ".").

## Syntax

```text
FEXT <VarName><=><FileName>
```

## Parameters

- **VarName**: to store the extension;
- **FileName**: a valid file name.

## Examples

```text
FEXT fExt=C:\Windows\System32\calc.exe or FEXT aExt=X:\Path1\Path2\FileName
```

## Remarks

- The outcome of example 1 is "exe",and example 2 is null;
- The returned value of FDIR,FDRV and FEXT can be checked or compared by the command of FIND
- Supportive: \\?\Harddisk0Partition2\A.TXT   \\?\HarddiskVolume5\A.TXT  \\?\ImDisk0\A.TXT

