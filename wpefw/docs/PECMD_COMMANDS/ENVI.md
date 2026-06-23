# ENVI    SET

To set or eliminate environment variables.  SET is used for PE variable.SET equals ENVI &,
SET-def and SET & equal ENVI &&.The others are the same as ENVI.-[f1:t1[:f2]] for vector.

## Syntax

```text
ENVI[.$#%~~.*=<>-x1-[Lfr:Lto[:Rfr:step]]] [&[&]|^][$][#][-][*][ ][[~Destructor~]Name][=Val]
ENVI[.$#%~~.*=<>] &::[Name][=Val]   //global PE variable
ENVI-copy PEVarName=SrcPEVarName[;SrcOff;Len;DstOff]  //copy binary object, []: part_copy
ENVI-cmp  PEVarName=SrcPEVarName[;SrcOff;Len;DstOff];[S|s|I|i]  //Compare,S/s/I/i:String,
          S/I:WCHAR, s/i:char,I/i no case,  S/s/I/i whithout Len means strcmp without len
ENVI-tom  PE_VarName=SrcPEVarName   //UNICODE to multibyte string(like GBK)
ENVI-tow  PE_VarName=SrcPEVarName   //multibyte string(like GBK)to UNICODE
ENVI-addr [AddrVarName][; LenVarName]=SrcPEVarName //to return memory address,length of byte
ENVI-swap  PE_VarName=PE_VarName2              //to swap the content of an object.
ENVI-make PE_VarName=[Addr|&PEVarName[@[$]Offset]][;[*][[$]ByteNum]]  //create a PE variable
    according to the memory address and length. * indicates a character string. $:N Byte
ENVI-mkdummy PE_VarName=[Addr][;[*][[$]ByteNum]] //the same as -make,but only create a dummy
        Addr may be  PEVAR[@[$]Offset] or (shm)ShmCtrName[@[$]Offset]   ByteNum=~:from obj
ENVI-mkfixdummy PE_VarName=[Addr][;[*][ByteNum]] //the same as mkdummy,but memory is fixed
object using the original memory.
ENVI-zero  PE_VarName=[[$]Val][@[$]Off]][;[$]Num]  //clear memory, default:0.$:N B [times]
//suffix-env //to temporarily cancel forcelocal so as to operate the environment variable.It
should be the first suffix.
//suffix-env //temporarily cancel forcelocal.Should be the first suffix.
//suffix-std //temporarily set EnviMode=1.Should be the first suffix.
//suffix-ret[TraceLevel]    //to trace a specified level(1 by default) and then operate the
PE_VarName,like the return of a function.
//suffix-get[TraceLevel]    //to trace a specified level(1 by default) and then obtain the
PE variable,like transfering a function to a PE_VarName.
//suffix-raw    //the part after = will not be interpreted.
ENVI-DataType  PE_VarName=Num[:[~]ByteNumOffset]    //to produce binary data of specified
type in PE object. BOOLEAN=bool=1 BOOL=enum=4
ENVI?DataType[0[@]] SrcPeVarName=VarName[:[~]BitNumOffset] //to obtain binary data of speci.
type in PE object. 0:to fill zero.@:to remove 0x. s: for sign.  ~: unit of type
//DataType:char wchar short int long longlong int64 float double ldouble ptr(pointer)
ENVI^ is exclusively used in control command.^ refers to the first suffix.
SET-def  ABC  //to ensure there is variable ABC of this level.If not,add a dummy variable;
If so,don't do anything.ENVI &&ABC also works.
ENVI-ex ReturnVarName=[&[&]]VarNameQuery //to check if such a variable exists.& indicates PE
variable,&& PE variable of this level,otherwise environment variable.
The first prefix before VarName ^ indicates an environment variable (ENVI ^ABC=1).Likewise,
%^VarName% is also an environment variable; It is out of the control of FORCELOCAL.
```

## Parameters

- **To specify the name and value of an environment variable.**
  "." indicates converting \n into newline.For a control,it indicates disabling newline."$"
  indicates converting the value into corresponding characters basing on hex,8 a group or
  separated by space; "%" indicates converting basing on decimal(default),separated by space
  ; Both are UNICODE. "~" indicates interpreting the variable once again; "~~" twice more.
  "*" indicates if there are many rounds of conversion based on "$""%",[*ReptFactor] refers
  to data.= indicates repeating according to the spaced word,[*ReptFactor] refers to a word.
  # coordinating with $% indicates being converted into corresponding bite/memory mode.
  <> aim at PE variables,< indicates appending at the back,while > insertion at the front.
  "$" indicates setting environment variables of system level and "#" of user level,both of
  which can be used simutaneously,broadcast all programs and wait for 5-second timeout when
  the system is busy.*- coordinate with $#.- indicates the THREAD broadcast backstage;
  * indicates only setting without broadcast.A later broadcast can be made by using ENVI.
  -x1 -x2 -x3 indicates interpreting the expression 1~N times.
  "&" indicates setting a function or PE variables of a window.
  "&&" indicates the function of defining,ie,defining or setting variables of current level.
  If such variables do not exist,they will be added.If exist,only set environment variables
  used in this program.Thus "&&" will also help to shield/protect variables of higher level.
  Environment variables without = indicates deleting them,like "ENVI ABC".
  PE variables without = means only clearing their content,like "ENVI &ABC".
  Set environment variables as null if their values are not specified,like "ENVI ABC=" or
  "ENVI &ABC=".
  "ENVI *"(Name and Val absent) indicates transfering the drive letter for CDROM to a system
  environment variable in the form of CDROM0,CDROM,CDROM1 and CDROM2.
  "ENVI $" indicates broadcasting the notification of changes after setting following system
  environment variables:
  Favorites      Favorites Directory
  Desktop        Desktop Directory
  StartMenu      StartMenu Directory
  Startup        Startup Directory
  Programs       Programs Directory
  SendTo         SendTo Directory
  Personal       MyDocument Directory
  QuickLaunch    QuickLaunch Directory
- **"ENVI" indicates informing all programs of the changes of environment variables in the**
  registry(5-second timeout,multithread or multiproess is applied to avoid feign death).
- **Destructor**: only effective the first time to define the PE variable."Destructor VarRef
  VarVal" will be called when the PE variable quits its definition range.

## Examples

```text
ENVI TEMP=%SystemDrive%\TEMP or ENVI &&ABC=123 or delete ABC:ENVI &ABC or ENVI$ NL=0d 0a
or ENVI$ DATA=*1M 30 0d 0a  or  ENVI= DATA=*1K 0x00  or ENVI$# &DATA=*1K 30
or SET-def ~ClosehandleX~h=0.As destructing,ClosehandleX &h %&h% or SET~-x1-[1:2] B%%d=A%%d
```

## Remarks

- Progams executed by "EXEC" will automatically inherit the environment variables of this
  PECMD.In other words,the environment variable set by "ENVI" without "$" in the config file
  is still effective in the subsequent "EXEC" programs.WinName and CtrlName can also carry &
- ENVI^ EXPORTLOCAL=1:to enable automatically inherting the PE variables of this PECMD.
- ENVI^ EXPORTLOCAL=0:to redisable automatically inheriting the PE variables of this PECMD.
  (default).
- ENVI^ EXPORTLOCAL=&1:to enable automatically inherting the PE variables of this PECMD,only
  effective for the current and subsequent level,used in thread.
- ENVI^ EXPORTLOCAL=&0:to redisable automatically inheriting the PE variables of this PECMD
  (default),only effective for the current and subsequent level,used in thread.
- ENVI^ FORCELOCAL=1:to force all variables to be PE variables.This is the simplest way to
  achieve muitithread or multitask(concurrent window).
- ENVI^ FORCELOCAL=0:to cancel the forcing above (default).
- ENVI^ EnviMode=0:mode compatible with 4.0,default.A dummy variable will not be
  interpreted.Variables are automatically interpreted non-sequentially in multiple rounds.
- ENVI^ EnviMode=1:standard mode.A dummy variable will be interpreted as null.Variables are
  interpreted in turn once, but times for VarName before =.   (suggested)
- The command can be applied to refreshing environment variables when params are absent or
  only with "$".Null character is permitted.Pay attention to the unnecessary null characters
- In standard mode,variables undefined are left dummy rather than uninterpreted(a status of
  uncertainty actually).
- Attention:Null characters ahead and behind are permitted,like TEAM ENVI ABC= 123  |...
  ABC becomes " 123  " instead of "123".
- "~" indicates that the variable will be interpreted once more,ie,after it is interpreted,
  the data will be used as the variable name with its value returned.
  "~." indicates that the data will be used as a param of a function with its value returned
  "~*" indicates being interpreted according to "~." if the variable is in accordance with
  the param format,otherwise according to "~"."~~" means being interpreted as this twice.
  "~","~~","~." and "~*" can be called repeatedly.
- Null character is permitted.Notice that null characters at the end will all be abandoned.
  like:ENVI  A=123          |  ENVI  B=123
  //Null characters behind A=123 are effective while the ones behind B=123 are abandoned.
- PE varibles are created and assigned by SET-def A=,SET &A= or ENVI &&A= (only assign if
  already existed) at current level; They are automatically destructed after quitting their
  range and cannot be deleted but cleared or modified to ensure their stable sharing.
- PE varibles have the effect of shielding,ie,varibles of lower level shield the ones of
  higher level with the same name.Be sure to use PE variables in multithread so as to avoid
  mutual interference.
- Access PE variables by %&VarName%,priority given to sublevel.When the function returns,PE
  variables of the level will be automatically released.Use SET directly for PE variables.
- Global PE variable:accessible and operable for all functions and threads,not automatically
  destructed and effective throughout the program running.
- PE variables behind the sharing point will be automatically copied to the subthread after
  a thread is created.
- Variables of windows and functions with lasting stacks (*function)are shared while those
  of the subsequent functions or blocks are not shared and they are automatically duplicated
  in the subthread.
- Programs executed by "EXEC" will not automatically inherit the PE variables of this PECMD.
  WinName and CtrlName can carry &.
- Value transfer and referencing transfer.Value transfer:It is the value that is transfered
  and it is one-way transfer from the caller to the callee,like %ABC% with a pair of %.
- Referencing transfer:It is the variable name that is transfered and it is two-way from the
  caller to the callee.Numeric values can be transfered back,like ABC without a pair of %.
  ◙Attention should be paid that referencing transfer supports binary inclucing the binary 0,
  which is not suitable for value transfer.
- To refresh the desktop: ENVI @@DeskTopFresh=[clearicon][;][1/2/4/8/16][;[-/+]PATH]
  //1:to refresh the desktop and the window of My Computer. 4:for change
  //2:to refresh the desktop professionally only.clearicon:to clear the icon cache(first).
  //8:to refresh ICO and file associations.
  //16[:func]:to refresh path(Desktop), -to remove,+to add,or refresh.
- To enable the tray menu:           ENVI @@TaskIcoMenu=1
- To disable the tray menu:          ENVI @@TaskIcoMenu=0
- To switch between tray menu enabling/disabling: ENVI @@TaskIcoMenu=2
- It is generally used for hotkeys of system level and it is the system tray to be operated.
  If it is used in a program,it is of program level and eliminated automatically when the
  program exits.
- HELP default color: ENVI^ HelpColor=[*CMDLineHeight] [ForeColor][#BkColor]      //global
- String breaking: ENVI^ Arg=*[BreakCharSet]BrokenStrings
  = being followed by * indicates that the breaking character between the broken strings
  will be treated as a dummy argument,or else abandoned.The params obtained by ENVI^ Arg=
  will overwrite the param list of the current function or LAMDA.The referencing of these
  params is the same as those of a function.
  BreakCharSet:If null,indicated as :sp:; Particularly,:sp: indicates null character;
  ] is indicated as \] and \ as \\;Variables are supported to transfer such characters.
  [] outside of BreakCharSet cannot be absent.If = is followed by *,the dummy arguments of
  odd number are treated as words,while the ones of even number are the constitution of
  BreakCharSet(null possibly).
- Clipboard: ENVI^ Clipboard=Str          //variables supported
- Clipboard: ENVI^ Clipboard?=VarName     //to query
- To query the version of a file: ENVI ?FileVarName[,ProduVerName]=FVER,FileName
- To obtain the name of a dragged file: ENVI ?[SingleName],[AllName]=DROPFILE,wParam
  The correspondant message is WM_DROPFILES.wParam is the first message param.
  SingleName indicates returning the first,while AllName returning all in multiline.
  Eg: ENVI @LABE1.MSG=0x0233::wp,lp, CALL OnDrop1 %wp%  %lp%
  _SUB OnDrop1
  TEAM ENVI  ?&&F1,&&FS=DROPFILE,%1| ENVI @LABE1=%&FS%
  _END
- To move the position of mouse: ENVI @@Cur=X;Y             //"; " or ",",the same below.
- Query mouse_position : ENVI @@Cur=?[@HWND;][[Nm]X][;[Nm]Y][;POINTNm][;KeyNm][;RcNm][;InNm]
- To query the position of mouse: ENVI @WinName.Cur=?[VarNameX][;VarNameY]
  [;VarNameCustCoordX][;VarNameCustCoordY]
- To query the position of mouse: ENVI @WinName.Cur=X;Y
- To display or hide mouse: ENVI @@Cur=*Num  //0:to hide,1:to display,should be paired.
- To query WinID and CtrlID: ENVI @WinName or CtrlName.ID=?[WinIDVarName][; CtrlIDVarName]
- To query the position of a file:ENVI ?[StartVarName][,LenVarName][,CluSizeVarName]
  [,DiskBlkListVarName][,BlkNumVarName][,PartStartVarName]
  [,DEVRetVarName]=FPOS,FileName
  DiskBlkList:indicating the distribution in partitions,one line for a block with its start
  and lenghth in it successively.Start -1 indicates special compression or hole.
  //65535 blocks at most; 65536 indicates being unfinished possibly,byte as the unit,-
  before BlkNum indicates the address relative to the 0 cluster.Start -38:for resident file.
- To query the window including the point: ENVI ?[Name][,TopName]=PWIN,[x],[y][,Flag]
  Flag:1=skip hidden  2=skip disabled 4=skip transparent 0:all default=3
- MouseCaptureA: ENVI @WinName or CtrlName.MouseCapture=1/0   // 1:enabled,0:released
- MouseCaptureB: ENVI @WinName.MouseCapture=#1/#0             // #1:enabled,#0:released
- ENVI @@EATEKEYS=CombKey1 CombKey2 ...  //to intercept keys,with the same format as HOTK,
  but null character is not permitted.
  like ENVI @@EATEKEYS=VK_LWIN  VK_RWIN ALT+VK_TAB  Alt+VK_ESCAPE  Ctrl+VK_ESCAPE
- ENVI @@RMENU=VarName; FileName        //to obtain the right click menu,multiline.
  The blank line generally indicates separators.
- Command Alias: ENVI^ Alias [*opt] CMDAlias=[CMDFrontHalf]  //the alternative command alias
  for the front half of the command.*opt:to optimize.
- Switch ENVI $# Broadcast:ENVI^ EnviBroad=0/1/-          //1:enabled(default); 0:disabled;
  -:backstage
- Switch &&__arg param list(compatible to the lite version):ENVI^ __arg=0/1   // 1:enabled;
  0:disabled(default)
- PECMD memory varible:ENVI^ memvar=[?RetVarName,][:ByteNum:]AddrOffset,Val   //to modify/
  query PECMD memory variable.
- PECMD memory varible:ENVI^ LoadEnvi [[HKCU\]SubPath|-] [VarName]    //to update [EnviVar]
  from registry,cautious!!
- To query parent process ID: ENVI ?RetVarName=PPID,ProcID
- To query if this process is an administrator. ENVI ?RetVarName=ISADMIN
- BkProgress: ENVI @WinName or CtrlName.percent=[Percent][RLCVEF][:BkColor][:PrgColor]
  [:TxtColor][:TXT]
- ENVI  @@percent=WinID:[Percent][RLCVEF][:BkColor][:PrgColor][:TxtColor][:TXT]
- Secret Mode(to clear after memory is out): ENVI^ zero=0/1 //0:disabled(default); 1:enabled
- To disable X64 Fs Redirection: ENVI^  DisX64=1,OldCfg //Two commands should be used in
  To restore: ENVI^  DisX64=0,%OldCfg% //pair and Varname remain the same without modifying
- To query version of windows: ENVI ?RetName=WinVer[+][;[^]*|File] //or ProductVer of file
  *:Kernel File, ^:FileVer, +:Fix WinVer with File
- To query version of windows: ENVI ?RetVarName=ispe
- To query program/system's bits: ENVI ?StrNm,NumNm=PEBIT,[FILE_System32Dir] //x86/x64/ia64
- Query firmware envi_var: ENVI ?[$.]RetName[,AttrNm]=FVAR,VName[;NameSP_GUID]
  non_pe_var or $: return HEX     .:wstring
- Operate firmware envi_var: ENVI ?[-v] [Attr]=FVAR+[$#%],VName[;NameSP_GUID],Val
  //empty val to delete.  -v:Val is name.   # byte mode. $ HEX. % dec mode.
  ENVI ?$&v=FVAR,BootOrder

