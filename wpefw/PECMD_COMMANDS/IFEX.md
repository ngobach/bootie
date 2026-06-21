# IFEX

If the conditional expression holds,command group1 will be executed; If not,command group2.
▲!Notice:"!" before <COND> indicates conditional reverse.^#~+- are the alternative
  delimiters of !.

## Syntax

```text
IFEX[|][^#~+-][*ab] [! ]<COND>,[CMD1][! CMD2]      //Take notice of the space behind !
      //*ab to exclude removable disks AB
IFEX{|}[^#~+-]{*ab} {! }{$}[{! }<COND1> relational calculus {! }<COND2> ..],{CMD1}{! CMD2}
      //combined conditions
IFEX  QureyCOND=?,VarName      //to query
```

## Parameters

  ☼COND:judgement to [MEMU],[DiskSpaceU],[FileDir] or [Key].
  ☼MEMU:MEM<ComparisonOperator>Num.The unit is M,ceiling(in one method).MEMU:usable memory;
  MEMA:all memory.
  ☼MEMU:MEMB<ComparisonOperator>Num.The unit is Byte.MEMBU:usable memory; MEMBA:all memory.
  ☼DiskSpaceU: R:\<ComparisonOperator>[*]Num,R:drive letter. (MB) *:Bytes
  ☼Key:KEY<ComparisonOperator>Num.
  ☼FileDir:the name of file and directory,wildcards are permitted.If ended with\,it should be
  a directory.
  ☼number comparison:$LeftPart<ComparisonOperator>RightPart,the numbers can be signed floats.
  # indicates integer comparison.
  ☼string comparison:|LeftPart<ComparisonOperator>RightPart
  or "LeftPart"<ComparisonOperator>"RightPart".
  ☼comparison operator:comparison operator "<",">","=",">=","<=","<>","&","|","@"
  respectively indicate being less than,greater than,equal to,greater than or equal to,less
  than or equal to,unqual to,and,or,xor.&|@ are only applicable to combined conditions with
  spaces at its front and back.
  ☼number:numbers to compare,integer and float are supported.The unit for the disk is MB and
  the key value is the key code.GMKS can be present.
  ☼CMD:a command or commands to execute.Commands are separated by "|" and groups are
  separated by "!".
  ☼IFEX|:group mode,it indicates checking the character "|" in the params behind.
  Not to check by default to avoid errors.
  ☼query conditions: MEM[UA]=?  MEMB[UA]=? R:\=? KEY=?Num
  ''indicates string comparison.[:eFileName] indicates checking the file name;
  [:pPrgmName] checking the process.

## Examples

```text
IFEX KEY=17,TEAM TEXT Searching system fonts…| FONT \WINDOWS!TEXT Installing add-on fonts
IFEX COND1,!! IFEX COND2,! IFEX COND3, DoSth1! DoSth2    //the nesting sentence pattern,can
be combined with FIND.
IFEX [ COND1 & ! COND2 & "a   b.txt" ], DoSth1! DoSth2   //example of combination.
IFEX [ $2 < 12 & C:\>10 & C:\ > 10 & ( C:\boot.ini ) & MEM > 100 ], MESS YES //example of
combination.
```

## Remarks

- This command is powerful but complicated and can be used in nesting(IFEX or FIND) to judge
  mutiple conditions.&|@ are relation operators.
- If this command nests FIND or IFEX,"!" should be used to separate the command groups in
  the nested command so as to avoid confusion.
  "," being closely followed by !! indicates there being no ELSE part for the convenience of
  nesting combination.The type before [ indicates all inside [] being of this type by
  default(Separate specification of type is also permitted inside).
- The command FIND is similiar to IFEX in function.
- If this command is used in the variable judgement,all variables will be treated as the
  type of double.
- combined conditions:[] are their symbols,and () can be nested.S| before[ indicate all
  behind being $| comparison.S| can be absent.|&@ are relation operators.There should be a
  space after the inner-condition.
- IFEX X: //to check if there exists the drive letter X only,likely a junk letter,though.
  (ie,though IFEX X:returns true,it will not exclude a junk letter),compatible with the
  virtual disk.IF IFEX X:\ returns true,it is certain that there is a file system in the
  drive,otherwise,there is no file system.
- Supportive: \\?\Harddisk0Partition2\   \\?\HarddiskVolume5\  \\?\ImDisk0\

