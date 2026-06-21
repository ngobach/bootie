# EXEC

To execute EXE,BAT and CMD programs.

## Syntax

```text
EXEC[|][*[1|N|-]] [-catch] [-cmd:[:ms:]CMD][-clone:VarName] [-[h]pid:VarName] [-no64]
    [-user:[BASE]UsrName [-passwd:[BASE]PassWd|-passwd*BASEPassWd]] [-limit] [-wd:[*]WorkDir]
    [-REALTIME] [-HIGH] [-ABOVENORMAL]  [-NORMAL] [-BELOWNORMAL] [-LOW] [-IDLE] [-mem]
    [-min] [-max] [-show]  [-wait] [-hide] [-doc:[mode]] [-hook] [-runs] [-job] [-u2d]
    [-raw] [-logs:[?][*]LOGSFile] [-nowin] [-shel:"AutoCMD"]
    [-exe:execfilename] [-err] [-err+]  [--]   NAME[+]=[=][!][*][@][$][&]<EXEPath>[Param]
EXEC -poprmenu[:x.y]        FileName //to popup the right click menu of the file.
EXEC -runrmenu:MenuItemName FileName //to execute the specified item of the right click menu
EXEC /InstallService name ⌠--gui-⌡[--delayservice ms] [--wait] [--nowait] [--nojob] [--1]
      [--hide] [--killwin Title] [--killwin2]  [--idle ms  [--idlewait]]    cmd_and_params
EXEC /RemoveService name
```

## Parameters

- **To specify the program path and params.The leading params are as follows(can be used**
  simutaneously regardless of the sequence):
  ☼-cmd:[:[*]ms:]CMD indicates the command to execute at a max/timer(*) time.1000ms by default.
  and the latter ":" can be absent if there is no confusion.It should be the first switch.
  ☼-limit indicates executing in a safe mode without administrator privileges;
  --job refers to the mode of job,coordinating with = to ensure the child process to be
  killed together.
  /REALTIME real-time priority,/HIGH priority of HIGH,/ABOVENORMAL priority of ABOVENORMAL,
  /NORMAL priority of NORMAL,/BELOWNORMAL priority of BELOWNORMAL,/IDLE /LOW lowest priority
  ☼-- indicates the end of switch -.It is only used when confusion arises between the switch
  and command;
  ☼-wait or "=" indicates waiting for the end of execution; -ex1:PE varibles of the closest
  level will be inherited as environment ones. -waiti  indicates waiting for GUI INIT.
  ☼-exe:execfilename indicates the name of an executable file,capabile of  executing files
  with nonstandard extension,like,myinstall.tmp.At this time,<EXEPath> behind is merely part
  of the command line.It can be arbitary,irrelevant and even absent;
  ☼-wd:[*]WorkDir indicates setting the working directory of EXE files,quoted if there is
  null character."*" indicates naming the file afer the directory;
  -wd:** refers to the directory containing the this EXE file itself.
  -wd:* refers to the directory containing the APP EXE file.
  ☼-code:<UTF8|ANSI|UNI|NUM> to specify the source code.
  ☼-pid:varname indicates storing pid in varname so that the process can be KILLed accurately
  -hpid:varname indicates storing handle of pid.
  ☼-hide,"!" or "*" indicates executing in a hidden way;
  ☼-show indicates executing in a visible way;
  ☼-min,"^" or "_" indicates executing minimally;
  ☼-max or "&" indicates executing maximally;                   ☼-u2d unix2dos
  ☼-doc[:mode] indicates document mode; Executed by the function of ShellExecute,some non-
  executable files (like *.TXT,.BMP etc.) can be opened; mode can be absent.
  Such operations as open,edit,explore,find,print,properties and runas are operable,as well
  as opening an EXE file.runas is used in installing programs demanding super privileges.
  ☼-user: -passwd: -passwd* indicating running a program with a specified account;
  ☼-err indicates capturing standard error output,-err+ capturing standard output and
  standard error output;
  ☼-mem indicates the ghost process executed in the memory;
  ☼-raw indicates the primitive binary data captured,            EXEC*  -raw
  ☼-nowin:no window(CREATE_NO_WINDOW)      ☼-nfb:to disable the cursor of waiting.
  ☼"$" indicates the document mode;
  ☼-io:to take over the input and output of the subprocess.
  ☼"@" indicates executing on the background desktop(WinLogon),totally hidden,and thus unable
  to interact with users,only applicable to registeration,like:
  "EXEC @PECMD.EXE CALL $SHELL32.DLL,DllInstall,#1,U";
  ☼-hook:to modify the shutdown code in a process(hook the function "ExitWindowsEx").It is
  advised to execute "EXEC --hook =EXPLORER.EXE," to modify the shell function of shutdown
  ,thus when executing "Startup->Shutting down system","PECMD.EXE SHUT" will be executed to
  shut down the computer;
  ☼-runs indicates programs automatically running by getting them registered in the key of
  RUNS(suggested).They are run by Shell after logining.If "=" is used,they will be
  registered in "HKLM\Software\Microsoft\Windows\CurrentVersion\Run",otherwise,in
  "HKCR\Software\Microsoft\Windows\CurrentVersion\Run";
  ☼The suffix | behind EXEC refers to the pipe mode,supportive of binary command pipe at this
  time,as well as such redirection symbols as >,>>,<,2> and >&.
  ☼EXEC*[1|N|-] NAME= indicates storing the standard output of the program in the specified
  variable NAME and forced to be in the waiting mode.1 indicates intercepting only the first
  line; N indicates elimenating the newline to emerge into one.- indicates elimenating the
  last newline,otherwise the output remains unchanged.If the data is of large amount,it
  should be stored in a PE variable(like &ABC).NAME can be @CtrlName,thus it is displayed on
  the control realtime.NAME+= refers to the appending mode.
  So it is with -catch:to store the standard output of the program in the specified variable
  NAME.
  ▲Notice:EXEC*1 ... to terminate the command being executed after receiving a line of
  message.
  ☼-clone:VarName:to clone a PECMD to run the script referenced by VarName.
  --exe:capabile of specifying another PECMD.EXE,coordinating with --logs:[?][*]LOGSFile;
  with ? indicates only currently-opened LOGS are valid.
  ☼-no64:For pecmd32,this will not lift the restrictions to the X64 file system.
  ☼-shel:"AutoCMD"  Executed as SHEL   -incmd:clone for inner cmd
  ☼-timeout:[#*]MiniSeconds[:ErrCode] wait process timeout, not kill if #.*include child
  ☼-su[a|e|c|d]:System privilege a:active e:environment c:skip uac. d:CWD -uac: =-suc
  ☼-svrsys[?]:Service switch desktop -svrusr[?]:Switch by user ID ?:Only in service script
  -svr-:disable switch
  ☼/InstallService:to install service with the name of "name"; /RemoveService:to remove
  service,only applicable to command line.In script,use EXEC =%MyName% /InstallService ...;
  --killwin:aiming at MESS-svr windows; --killwin2:aiming at MESS-svr2 windows;
  --wait:to wait for the end of the process(default),--nowait:not to wait;
  --idle ms:to execute the command after being idle for ms;
  --nojob:not to kill the subprocess; --gui-: not INTERACTIVE --desk[-]:[not] switch display
  --idlewait:not to terminate the running program if there is input,otherwise terminate if
  there is input; --usr:switch user ID
  Executive body HERE_IS_SERVICE0:Service params can be embeded behind,100 characters.
  ☼To execute the embeded program,use --exe:[*[*]][?.TempFileExt:][cab:]Transfer[:CABSubFile]
  or transfer via the first word of a command.Without * indicates being directly executed in
  memory; With * indicates being executed in a temporary file to be automatically deleted.
  The figure refers to the ID number of resource in EXEDATA,compressible in CMPS.
  If there are many programs,the latter(executed in a temporary file) should be adopted.
  &PEVarName,ud:UDFileName:UDInternalFileName,ex:self-extracted packet,** indicates deleting
  more safely.?\ inidcates replacing a directory,and ?? replacing a FileName.
  Resources can be witten as:"[FileName]""#RsrName[|DirName]",and the latter pair of "" can
  be absent.The RsrName can be non-figure characters at this time.

## Examples

```text
EXEC =!CMD.EXE /C "DEL /Q /F %TEMP%"
EXEC*1 &&LABEL=!CMD.EXE /C dir C:\......
EXEC*N &&VER=!CMD.EXE /C VER
EXEC* &&DIRT=!CMD.EXE /C DIR /S T:\xxxxx 2>&1
EXEC* @Edit1=!ping 127.0.0.1
EXEC| =!ipconfig > ip.txt
EXEC| =*ipconfig | =%Myname%  TEAM READ --,**,V|| MESS. [%%V%%]  //ENVI^ EnviMode=1 first
EXEC|  !cmd /c dir  | !=gzip.exe -9 > dirs.gz
EXEC|*  &&V=!gzip.exe -d < dirs.gz
PECMD.EXE EXEC /InstallService IdlePoweroff --nowait --delayservice 20000 --hide
                         --idle 1200000  shutdown -f -s -t 30
EXEC* --exe:#100 &&V=cmd /c dir  //to call the embeded cmd.exe,directly executed in memory.
EXEC --clone:&MyStript   Params A  B  C
```

## Remarks

  SHELL is also loaded by this command;
  In the waiting mode,%&&ERRORLEVEL% is the exit code of a program; %ERROR% %&&ERROR% share
  the same value.
  In the waiting mode,programs can be directly executed as a batch without EXEC,like NOTEPAD;
  If there are no command word LOAD/EXEC but exist files with such extensions as WC?,IN? or
  TX?,they will be executed by LOAD,otherwise by EXEC;
  When a switch conflicts with a leading character,the last is valid.Between the leading
  character and file name,a space can be added to break;
  In the document mode,if params are demanded,the file names should be bracketed with ",or all
  of them will be treated as a single file name.
  If there are spaces in the program path and the file path in the parameter, use "".
  In the command line or batch script,! * --hide is generally ok without @,for @ is not
  supported by all versions of Windows.
  Detailed explanations to example 2 "EXEC* DIRT=!CMD.EXE /C DIR /S T:\xxxxx 2>&1":
  1:"2>&1" are the special symbols for CMD.EXE,indicating merging the standard output of error
  (2) into the standard output(1). EXEC accepts the standard output by default,thus it accepts
  all the content.
  Command:C:\>dir XYAXXX
  The volume in drive C is WINDOWSXP
  with its serial number 28F0-182C
  Files are not found in the directoy of C:\
  //Here "Files are not found" is a standard output of error(without 2>&1,there will be no
  such a line),while others are standard output.
  EXEC*  --err   indicates accepting a standard output of error.
  2:1 indicates only intercepting the first line.
  This is because some commands output too much,yet only the first line is needed.This helps
  to terminate the program and return the outcome at once.
  3:to accept other lines of output.Firstly transfer all to the variable &V,and then intercept
  the specified line from it.
  1>  FORX  *NL &V,&line, ....
  FORX automatically splits the output into lines and circulates every round.&line refers to
  a line. //[HELP FORX]
  2>  use READ   //[HELP  READ]
  READ  -,-1,&LINES,%&V%         //to read the number of lines.
  READ  -,0,&line,%&V%           //to read the last line.
  READ  -,1,&line,%&V%           //to read the first line.
  READ  -,2,&line,%&V%           //to read the second line.

