# READ

To read the data of a specified file in string(line).Other code:-GBK -BIG5 -UNI -UNIBE -ANSI
-CodeNum

## Syntax

```text
READ[-UNICODE|-UNICODEB|-UTF8] [*fix] <FileName>,[ReadPos[+LineNum]],[VarName]
READ [*fix] [-[*]],<ReadPos[+LineNum]>,<VarName>,<ContentRead|ContentReadVarName>
READ [*fix] --,<ReadPos[+LineNum]>,<VarName>
```

## Parameters

- **FileName**: an existing file,supporting environment variables interpretation; *fix:a separate
  ENTER is treated as the newline(encrypted BAT/CMD).
- **ReadPos**: the line ID,<0 indicates reading the number of lines; 0 indicates the last line;
  Other numbers indicate reading the specified line,hex numbers supported,1 by default.
  * or ** or *r indicates all lines; * indicates the newline serving as the interval; *r
  indicates the newline via ENTER as the interval,both of which can be mutually converted.
  ** indicates the newline of the primitive content.The number of lines is 1 by default,and
  0 indicates reading to the end.
- **VarName**: a string to store the outcome of reading.READLINE by default.
- **ContentRead**: to read from the content of the multiline string,which is generally obtained
  by EXEC* or assembled by ENVI.FileName is - or absent or -* at this time.-* indicates this
  being VarName to read(?check code).-UNICODE -UNICODEB -UTF8 are the file encoding,which
  can be simplified as -UNI -UNIB.If BOM exists,the encoding is based on BOM.
- **Format 3**: to read from the standard input.Generally,the position is 1 and one line is read
  each time.

## Examples

```text
READ %CurDir%\Memu.LST,10,CurLine or READ %CurDir%\Memu.LST,0,LastLine
READ  -,10,Line,%&MSG%    or    READ  -*,10,Line,&MSG
```

## Remarks

- READ is only applicable to reading a pure text file like the menu config file.To read the
  the primitive content of a file,use the command of GETF.
- Supportive of UD files: ud:UD FileName:UD internal file name.
- Supportive: \\?\Harddisk0Partition2\A.TXT   \\?\HarddiskVolume5\A.TXT  \\?\ImDisk0\A.TXT

