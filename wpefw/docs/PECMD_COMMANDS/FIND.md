# FIND

If the conditional expression holds,command group1 will be executed; If not,command group2.
--sub:to recurse.^#~+- are delimiters.
▲!Notice:"!" before <COND> indicates conditional reverse.$ before [ indicates all behind
  being $ comparison and it can be absent; Relational operators are &|@(xor).

## Syntax

```text
FIND[|^#~+-][*c] [! ]<COND>,[COND1][! CMD2]   //Take notice of the space behind !, ^#~+- are
  the alternative delimiters of !.
FIND{|^#~+-}{*c} {! }{$}[{! }<COND1> relational calculus {! }<COND2>  ..],{CMD1}{! CMD2}
  //combined conditions
FIND  QureyCOND=?,VarName                     //to query
FIND  --pid*@.[#ParProcID|ProcID]  VarName,PrgmName  //to query,* means matching the front
  part only,"." means matching the extension.
FIND [--sub][--forpid:PID|--fortid:TID] --wid*@[ParWinID] VarName,WinTitle//to query,* means
  matching the front part only.
FIND [--sub][--forpid:PID|--fortid:TID] --wid#ParWinID VarName,CtrlID //to query WinID of
  the control.
FIND --menu    VarName,WinID                  //to query MenuID of the window.
FIND --menu#0  VarName,MenuID                 //to query SubMenuID of the Menu.
FIND --menu#SN VarName,MenuID                 //to query information of SubMenu with the
  specified SN,SN>=1.
```

## Parameters

- **COND**: judgement to [MemSum],[AllDiskSpace],[MemProc] or [Key].
  ☼MemSum    MEM<ComparisonOperator>Num.The unit is M,ceiling(in one method).
  ☼MemSum    MEMB<ComparisonOperator>Num.The unit is Byte.
  ☼AllDiskSpace  R:\<ComparisonOperator>[*]Num,R:drive letter. (MB) *:Bytes
  ☼Key       KEY<ComparisonOperator>[#]Num.
  ☼Proc   the name of a memory process(including *?).
  ☼string comparison:*Left_PEVAR_Name<ComparisonOperator>Right_PEVAR_Name
  ☼string comparison:$LeftPart<ComparisonOperator>RightPart,not case sensitive.The LeftPart
  and RightPart support variables.If an operator is contained in the LeftPart or RightPart,
  it is required to be transfered to a variable.
  For the convenience of being directly compared with an operator itself,the first character
  after $ will not be treated as an operator but the LeftPart of the comparison.
  ☼number comparison: |LeftPart<ComparisonOperator>RightPart,the numbers can be signed floats
  # indicates integer comparison.
  ☼operator:comparison operator "=","<>","<",">",">=" and "<=" respectively indicate being
  equal to,unequalto,less than,greater than,greater than or equal to,less than or equal to.
  The suffix *c indicates case sensitive.
  ☼number:numbers to compare,the unit for disk is MB and the numberic value of the key is the
  code of it.GMKS can be present.
  ☼command group: a command or commands to execute.Commands are separated by "|" and groups
  are separated by "!".
  ☼FIND|    Group mode,it indicates checking the character "|" in the params behind.Not to
  check by default to avoid error.
  ☼query conditions: MEM=?  MEMB=? R:\=? KEY=?Num
  ☼--wid @ indicates all,one line for each. SN WinID CtrlID ParWinID ThreadID ProcID Type
  Title,separated by TAB.
  ☼--menu#SN: SN MenuID CtrlID TXT,separated by TAB.To return all if SN is null,in multiline.
  ☼--pid @ indicates a list,one line for each.ProcID ParProcID MemK CPUTime(100ns) TotalTime
  FileName CMDLine.CPUTime of process 0 refers to IdleTime of the system.Separated by TAB.
  ☼FIND --pid VarName   //to return: IdleTime TotalTime CPUNum ClkNum_For_OneSec
  100nsNum_For_OneClk
  --[in]visible :Windows is [in]visible. --class:WinClassName
  *var Use PEVAR_Name in COND

## Examples

```text
FIND MEM<128,SHEL %SystemRoot%\SYSTEM32\XPLORER2.EXE!SHELL %SystemRoot%\EXPLORER.EXE
FIND $%m%=1,!!  find $%n%=1, mess aaa!mess bbb         //example of nesting.
FIND COND1,!! FIND COND2,! FIND COND3,DoSth1! DoSth2   //nesting sentence pattern,can be
combined with IFEX.
FIND [ C:\ > 100 & $"A " <> "A" & explorer.exe ],DoSth1! DoSth2    //example of combination.
FIND --pid*.123 V,or FIND --pid*@.123 V,               //to check PID 123.
```

## Remarks

- This command is powerful but complicated and can be used in nesting(FIND or IFEX) to judge
  mutiple conditions.
- If this command nests FIND or IFEX,"!" should be used to separate the command groups in
  the nested command so as to avoid confusion.
  "," being closely followed by !! indicates there being no ELSE part for the convenience of
  nesting combination.
- The command IFEX is similiar to FIND in function.
- The key value can be A~Z,0~9,code 0xNN or numeric code with at least 2 digits.# can be
  absent.It is just for compatibility.With # indicates it should be in the code mode.
  VK_ sequence is applicable to the key,whose outcome is stored in the environment variable
  %PressKey%.
  The key makes sense only with =,which checks the specified key(not to check other keys,
  refer to WAIT).
  To compare with the separate = itself,= can be placed behind $.In some complex situations,
  variables should be employed to store some special characters for their referencing.
- --forpid:PID --fortid:to specify PID or ThreadID respectively that the command belongs to
- combined conditions:[] are the marks,can be nested,but () are suggested.$ before [
  indicates all behind being $ comparison.Relational operators are |&@,separated by space.
- Supportive: \\?\Harddisk0Partition2\   \\?\HarddiskVolume5\  \\?\ImDisk0\

