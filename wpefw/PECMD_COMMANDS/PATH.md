# PATH

To set the current working directory for PECMD.EXE,create or delete a directory.

## Syntax

```text
PATH [#|@]DirName
```

## Parameters

- **DirName**

## Examples

```text
PATH %CurDrv%\outpart
```

## Remarks

- "@" indicates setting the current directory for PECMD.EXE so that the relative path can be
  adopted by the commands of EXEC,DEVI and so on to shorten the length of the command line.
- "#" indicates deleting a directory.Notice that this operation is also likely to delete a
  file with the same name.
- If "@" and "#" are absent,PATH is used to create a directory(multilevel).
- Be cautious to use Path with the leading character "@" in a config file,which is likely to
  result in disorders.
- If the param is absent,the current default directory of PECMD.EXE(%SystemRoot%\SYSTEM32)
  will be restored.

