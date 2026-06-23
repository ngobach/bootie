# FDIR

To return the name of the directory containing the specified "FileName"(without "\" at end).

## Syntax

```text
FDIR [--short[+]|--long[+]] [--full|--fullfile] [--volume] VarName=FileName
```

## Parameters

- **VarName**: to store the directory name; --full refers to an absolute directory,ending with \
  indicates itself,--fullfile indicates full path of the file.
- **FileName**: a valid file name,-short/-long:to export a short/long file name(+if have).
  --volume:to export a file name with volume.   --1 add double quotes on spcae
  --part:to export \\?\\Harddisk#Partition#    --volumepart:--volume then --part

## Examples

```text
FDIR fPath=C:\Windows\System32\calc.exe or FDIR aPath=%CurDir%\Path1\Path2\FileName
```

## Remarks

- If FileName is a relative path,the returned outcome is the working directory of PECMD.EXE
  or config file;
- The outcome of example 1 is "C:\Windows\System32" and example 2 is"%CurDir%\Path1\Path2".
- Supportive: \\?\GLOBALROOT\Device\Harddisk0\Partition1\A.TXT (Others are the same)
  \\?\Harddisk0Partition1\A.TXT   \\?\HarddiskVolume1\A.TXT  \\?\ImDisk0\A.TXT

