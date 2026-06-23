# SHUT

To shut down or reboot the computer.
  -force:to shut down quickly.

## Syntax

```text
SHUT [-force] [E|ONum|C|R|L|H|S|SHUTDOWN|-]  [--] [ScriptParamList]
```

## Parameters

- **No params**: to shut down the system by default;
- **"E"**: to eject CDROM first and then wait for 10 minutes;
- **"ONum"**: to eject CDROM first and wait for the specified number of MS,10000ms by default.
- **"C"**: to shut down CDROM;
- **"H"**: to be in dormancy.It should be supported by the system(enabled in the power option in
  the control pannel);
- **"L"**: to logout;
- **"R"**: to reboot the system;
- **"S"**: to suspend the system;
- **"K"**: to lock the computer;
- **"SHUTDOWN"**: offbeat shutdown.With params -s:to shut down(default) -r:to reboot -f:to force
  (default) --f:to cancel -f,-t:number of seconds

## Examples

```text
SHUT H
```

## Remarks

- That the command of "EXEC" hooks the specified function "ExitWindowsEx" and coordinates
  with SHUT will intercept all calls for shutting down the system so that it is properly
  done(Notice that this is a quick shutdown which may not save all the data).
- If drive letters cannot be timely assigned to such devices as u disk and USB removable
  hard disk or shutdown appears slow,please try restarting AudioSrv(effective to some
  SIGMATEL HD series of integrated sound card)
- TEAM EXEC =PECMD SERV !AudioSrv|SERV AudioSrv
- REGI HKLM\SOFTWARE\PELOGON\FORCESHUTDOWN=#1  //With such setting,shutdown in StartupMenu
  is a quick one.
- Automatically executed on shutdown:%SystemRoot%\System32\OnShutdown.wcs OPCode
  ScriptParamList
  OPCode:shutdown,reboot,logout,suspend,hiber,poweroff,unknown lock
  The shutdown menu supports:shutdown,reboot,logout,poweroff,unknown

