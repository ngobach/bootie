# DIR

To get a file list,one line for one file.

## Syntax

```text
DIR [--wd:[*]WorkDir] VarName params...
```

## Parameters

  ☼--wd:[*]WorkDir indicates the working directory to set the commands to execute.Bracket it
  with double quotes if there is null character."*" indicates naming the file with the
  directory name;

## Examples

```text
TEAM DIR &INILIST /s /O:N C:\*.INI| MESS. %&INILIST%@#OK
```

## Remarks

  For params behind,please refer to the DOS command DIR.The param B is implied in them.

