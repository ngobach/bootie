# SIZE

To return the size of a specified "FileDir",byte as the unit.

## Syntax

```text
SIZE [-link] <VarName><=><FileDirName>
```

## Parameters

- **VarName**: to store the size of file and directory.   -link:follow link file
- **FileDirName**: a valid name of file and directory.

## Examples

```text
SIZE iSize=C:\Windows\System32\calc.exe or SIZE iSize=C:\Windows\System32
```

## Remarks

- If FileDirName is a relative path,it is relative to the working directory of WinCMD.EXE
  or the config file.
- The outcome of example 1 is the size of "calc.exe" and example 2 is the size of the
  directory "C:\Windows\System32".

