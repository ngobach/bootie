# CALL

To call DLL function or subprocess.

## Syntax

```text
CALL $[? ][--cd ][--nrcd ][--c ][--ret:RetVarName ]DLLPath|*DllHandle[,FuncName][,[#]Param1]
      [,...[,[#]Param20
CALL $[? ][--cd ][--nrcd ][--c ]--ret:* DLLPath,FuncName,[RetVarName][,[#]Param1]
      [,...[,[#]Param20
CALL $--ret:RetName [--cd --nrcd ] ,-LoadLibrary,[^]DLLPath  //Load Dll ^:AutoFree
CALL $--ret:RetName [&DllMemVar],-LoadLibrary,*[FileName]#SrcID[|Cat]//Load MEM Dll AutoFree
CALL $--ret:RetName ,-GetProcAddress,*DllHandle,FuncName                //Get Address of fun
CALL $[--ret:RetName] ,-FreeLibrary,*DllHandle                           //Release DllHandle
CALL $--win [--qd@] [--cd ][--nrcd ] [--ret:RetVarName ]DLLPath,FuncName,CMDLineParam
                                                                      //to run dll32.
CALL $--cpl CPLPath,                                                  //control panel file
CALL [LeadChar]<SubprocName/WinName> [CMDLineParam]
CALL * <SubprocName>  [CMDLineParam]                   //call of THIS type
CALL <LeadChar><WinClsName:WinRealName> [CMDLineParam] //to generate an instance of a class.
CALL @-sub[:@-#LxTyWwHh]] [LeadChar]<WinName> [CMDLineParam]          //external subwindow
     @:to keep the relationship of parent and child; -#:frame with title
CALL @--popmenu[:x.y[:align]]] <WinName> [ParamList]    //popup menu
CALL --mem VarName [*] [CMDLineParam]          //to run the dynamic memory function code.
```

## Parameters

  ☼The leading Character "$" indicates calling DLL function,--c:C call regulations.If the
  function name is *,the call is only DLL preloading(Later calls will be faster),and it is
  safe to call many times.DLLPath,FuncName and params can be specified.If FuncName is absent
  ,it will call "DllRegisterServer".Params are UNICODE string by default."@" refers to
  narrow string and "#" refers to integer; 20 function params(100 for C call)at most.
  --cd:to shift to the target directory before execution; --nrcd/--rcd:(not)to restore
  directory after execution.Unkown DLL is executed in another PECMD thread.
  --qd:all led by leading characters,#integer,*PE_VarName,$string,=primitive string.
  --qd#/--qd*/--qd$/--qd@/--qd=/--qd~ :All params are integer/PE_VarNm/str/NarrowStr/UTF8Str
  primitive string;  --m:to run DLL(CMPS permitted)in the memory.
  DLLPath can be *[FileName]#SrcID(CMPS permitted;handle as *num
  --co/--nco:to initialize DLL(default) when it is registered/or not.
  --1:the whole part behind as one param.  --16:return Hex.
  --bool: API return BOOL on succ    --sret return signed int
  To call the address:If DLLPath is the character #,FuncName is directly the function address.
  To query the address:? indicates querying the address,the outcome stored in RetVarName.
- **"@"  indicates calling a window defined by _SUB.In a window defined by _SUB,execute CALL @**
  to call another.
- **"@*" indicates calling a window defined by _SUB concurrently,enabling it to be**
  simutaneously operated together with its parent window.The next command cannot be
  executed before it is closed.
- **"@-" indicates calling a window defined by _SUB in the background and continue with the**
  next command,but some time-consuming operations will still be blocked.
- **"@~" indicates calling a window defined by _SUB backstage and some time-consuming**
  operations will not be blocked.
- **"@+" indicates calling a window defined by _SUB backstage and some time-consuming**
  operations will not be blocked.The window becomes abandoned.The program does not wait
  for its being closed to exit.
- **"@^" resembles @*,yet the parent window will not shield the child,applicable to a purely**
  event-driven parent window.
  ☼"$"  Being absent indicates calling subprocess,whose name is the param.
- **The format of the function params**: In a subprocess and window,%0,%1,%2,...%n respectively
  refers to SubProcName/WinName,and the param No.1,No.2,...No.n.
  %# indicates the number of params.%* indicates all params starting from %1.
  %~0,%~1,%~2,...%~n indicates elimenating the outermost double quotes.
  %@ indicates all params starting from %0.
  ☼CALL can absent,together with @,eg:CALL @WIN1 a b c  or  WIN1 a b c
  ☼Popup menu:x.y are the menu coordinate.If absent,they are the position of current mouse.
  WM_MENUSELECT is hovering message for menu,with index：(%wParam%&0xFFFF)-%PE_MENU_IDBASE%
  Select:%&__MenuId% %&__MenuInx%
  ☼"CALL @--":to unload the windows environment."CALL @":to initialize the windows
  environment so that soem related windows commands can be executed in any place.

## Examples

```text
☼To call DLL function: CALL $SHELL32.DLL,DllInstall,#1,U
☼To call subprocess: FIND MEM>127,CALL EXPLORER_SHELL!CALL CMD_SHELL
```

## Remarks

- DLLPath: maybe *DllHandle  or  **MemDllHandle
- To call DLL function:Functions in DLL are in accordance with PASCAL call regulations,
  unless with --c(just ignore it if you do not understand).
  -DllRegisterServer: RegisterDLL  -DllUnregisterServer: UnregisterDLL
- To call subprocess:The command of CALL can only call the subprocess in the same config
  file,and this function is not applicable to the command line.      Refer to _SUB and _END.
- Common functions can also be called as window objects by CALL @,generating many objects
  (fiber).Use "KILL \",or "ENVI  @FuncName=Name1"(CALL command line by default) to destruct
  objects internally and use "KILL \Name1" to do so externally.
- In format4,THIS refers to the real window.
- In format2/4,if a param contains a null character,it should be bracketed by double quotes,
  or the param will be separated.
- %&&__LastWinID% is the unique ID of the last-created window and %&__WinID% is the ID of
  the current window.
- Call of THIS:The function runs in the stack of its caller,mainly to transform the &&
  initializing codes into a function.

