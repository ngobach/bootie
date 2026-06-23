# LOAD

To execute the command in the config file one by one.  //Notice:The null character in the
file name should be bracketed with "".

## Syntax

```text
LOAD [-del] [--m] <FilePath|*[FileName]#ScriptID[|Dir]>[|Offset[|LenLmt]][*FuncName]
     [,StartDrvLetter]                                                    [ParamList]
LOAD <*[FileName]##InternalScriptID[:Dir]>[|Offset[|LenLmt]][*FuncName] FriendlyAlias
     [Param]                                                    //friendly type
LOAD [--logs:[*]logFileName]  --mem VarName [CmdLineParam] //to run dynamic memory file code
//switch --logs:[*]logFileName]         //switch --Local :Forcelocal=1 Mode
//switch --EnviMode:EnviMode=1 Mode     //switch --ncd:not to transfer the current directory
                                          or set environment variables(multithread).
//switch -:not to transfer the lasting stack.
//switch -this or -*:not to transfer the lasting stack or executing stack.
```

## Parameters

- **specified FileName(containing path,supporting environment variables)and params.**
  Offset:start from this.The Offset and LenLmt support TGMKS.
- **-del**: to delete the script after loading it. --m \:to execute all sought files in the
  partition one by one.--m absent,to execute the first sought file only.
- ***FuncName**: to call the function in the file and exit.
- **|Dir**: If Dir is a string of pure numbers,it should be bracketed with a pair of " or ' or
  transformed to the format as ":Dir".

## Examples

```text
LOAD \outpart\PECMD.INI  A B or LOAD #102 A B or LOAD "X:\Program Files\a.ini" "1 x" 2
LOAD --mem &MyStript A B C           For FileName #123, LOAD   #123 p1 should be written as:
LOAD   .\#123  p1
```

## Remarks

- Each command is in a separate line and commands of the main process are executed totally
  in proper order,ignorant of invalid ones or blank lines.
- The config file supports the whole-line note and post-line note.For the former,"//"
  (suggested),"; " or "`" can be used and the latter," //" or "; " or "`" can be used.
- In a config file,the environment variable "%CurDir%" indicates the current directory and
  "%CurDrv%" the current drive letter.
- If the first non-quote character in FilePath is "\",the command will search the specified
  directory on all the disks for the file,like "LOAD \MyWinPE\PECMD.INI".
- If the directory containing the config file is set as the current directory,when a
  shortcut is to be created,a relative path can be used for the DestPath of the shortcut.
- For others,refer to MAIN.Offset[+LenLmt] makes it possible to run devices or fragments of
  non file system and merge scripts.
- LOAD supports specifying a character set for file encoding.Before the compression of CMPS:
  If BOM exists,the compression is based on it,otherwise the charater set is specified in
  the first line with "#code=CharSetNum"; If the first line begins with #!,the character set
  should be specified in the second line.BOM includes UNICODE,UNICODEBE and UTF8.
- friendly type:to specify FriendlyAlias as %0 instead of RsrcName containing #.
  The resource directory of script ID is SCRIPT by default.
- The FilePath or a certain param should be bracketed with a pair of double quotes if they
  contain null characters.Otherwise,they will be separated.
- If the command word LOAD is not absent,the first literally successive string will be
  treated as FileName regardless of the possibly existing variable of null character(the
  content of the variable contains null character).The corresponding double quotes around
  the string can be absent at this time.(compatible with ver4.0).
- Please adopt the command line standards of windows and linux as the ones for writing
  commands,which are also the standard writing practice for the command of LOAD.
- To avoid confusing FileName with #ScriptID behind,FileName can be bracketed with quotes.
  FileName can be non-figure characters.

