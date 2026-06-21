# WRIT

To modify the data of a specified file in the form of string(line).*fix:to treat a separate
ENTER as the newline. *-nl:not to append newline behind. *v:to write the variable. *fv:from
the variable. *c:Clear at firstly.

## Syntax

```text
WRIT[-UNICODE|-UNICODEB|-UTF8] [*fix] [*-nl][*v] FileName,[ModiPos],[FileData]
```

## Parameters

- **FileName**: valid file name,supportive of environment variables interpretation; - indicates
  standard output,-- standard error output,regardless of the position.
- **ModiPos**: consisting of "$","+","-" and Line ID with the format as "[$][+|-]<LineID>"; +0 by
  default.
  "$":to unfold the environment variables in "FileData";
  "+":to insert a line in the position of specified line ID with "FileData";
  "-":to deleted the line with the specified line ID;
  "+" and "-" absent:to replace the specified line with "FileData";
  Line ID is indicated as a number, "0":to operate to the last line.If line ID is set
  unproperly,WinCMD will not perform any operation.
- **FileData**: string,data to insert or replace a certain line in the file with. ModiPos led by
  "$" indicates supporting environment variables interpretation.
  -UNICODE -UNICODEB -UTF8 are the file codes.If there exists BOM,the encoding is based
  on BOM.Other codes:-GBK -BIG5 -UNI -UNIBE -ANSI -code number

## Examples

```text
WRIT C:\BOOT.INI,+0,C:\AVLDR.NT6=Window_7_PE or WRIT C:\BOOT.INI,+0,C:\AVLDR.NT5=Window_PE
```

## Remarks

- WRIT is only applicable to modifying pure text files like CMD,BAT and menu config file;
  CONOUT$ indicates testing the terminal.
- WRIT and LOOP should be in a separate line and should not be nested in such command as
  FIND,IFEX and TEAM.
- Supportive: \\?\Harddisk0Partition2\A.TXT   \\?\HarddiskVolume5\A.TXT  \\?\ImDisk0\A.TXT

