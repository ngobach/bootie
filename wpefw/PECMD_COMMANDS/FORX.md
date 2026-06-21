# FORX

To operate the matched file and directory with corresponding command.Its function is similar
to the command of FOR in CMD.EXE.

## Syntax

```text
FORX [*ab] [*cur] [/S[:SubDirLvlNum]] [/O:N] [/O:-N] [/size:Min:Max:Align] [@|$][[!]\]File,Va
     r,[MaxTimes],CMD_and_Param
FORX *[v] VarList[Name],<Var>,<CMD> [ParamList]
FORX *NL[:Separator] PE_VarName with lines of its content,<Var>,<CMD> [ParamList]
```

## Parameters

  *ab:to exclude AB removable disks;  /S:to search the subdirectory;  /O:N to sort by name;
  /O:-N to sort in the inverted sequence.*cur:to give priority to the current drive letter.
  *qu[~] surport "   *off to return var_part   *bf Breadth first
- **File**: to specify the file and directory,wildcards applicable.
- **Var**: to specify the name of a variable to refer to the file found.The name of the variable
  cannot be duplicated and the variable can be referenced in many places in the param list.
- **MaxTimes**: to operate the matched file and directory for the maximum number of times.
  <=0 indicates operating all the existing files.
- **CMD**: a valid command of WinCMD.EXE.The format and number of its params are determined by
  the command itself.
- **PE_VarName with lines of its content**: to store lines of content in the variable.Notice
  that there is no % if it is only a variable name.One line of its content is returned each
  time and it should be PE variable name only.

## Examples

```text
♦FORX %CurDir%\Path1\*.DLL,AnyDLL,0,CALL $%AnyDLL%
♦FORX \auto*.INF,AutoRunVirus,0,FILE %AutoRunVirus%
♦FORX !\Tools\Tools.INI,MyIni,1,LOAD %MyIni%
♦FORX /S /O:-N C:\windows\*.INI,f,0,MESS %f% @#OK
```

## Remarks

- This command can find the file and directory with attributes,like the ones with hidden
  attributes.
- "\" indicates searching all partitions."!" indicates searching all partitions in the
  inverted sequence.
- "@" indicates searching only the directory."$" indicates searching file and directory,and
  file only if absent.
- "*" indicates being in the mode of variable list,separated by null characters.
- Example 1 indicates registering all the DLLs in %CurDir%\Path1\.
- Example 2 indicates deleting all the auto*.INF in the root directory of all partitions.
- Example 3 indicates searching all partitions in the inverted sequence for Tools.INI in the
  directory of Tools and loading the first sought Tool.INF with the command of LOAD.
- If CMD and param are absent,the variable will return the number of files and directories
  found or the maximum number of times the command is executed.If CMD is *,return list.
- Supportive: \\?\Harddisk0Partition2\*.*   \\?\HarddiskVolume5\*.*   \\?\ImDisk0\*.*

