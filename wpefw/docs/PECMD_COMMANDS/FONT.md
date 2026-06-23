# FONT

To (un)register fonts or add on fonts

## Syntax

```text
FONT [-] <FontPath>[,StartPart]
```

## Parameters

- **FontFilePath and StartPart**

## Examples

```text
FONT %CurDrv%\OutPart\FONT or FONT \WINDOWS or FONT \WINDOWS,C:
```

## Remarks

- If the first character of the command is "\",it will search all the partitions starting
  from the specified starting partition for the fonts in Windows\Fonts,install and get them
  registered.Defining a starting partion helps to avoid searching the floppy in the computer
  If the starting partition is absent,the command will search all the partitions (including
  the floppy).

