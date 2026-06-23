# SERV

To enable,disable or query service or drivers.

## Syntax

```text
SERV [-wait] [Switch][?RetVarName] [!]<SrvName>
SERV-create [?RetVNm] SvrName,binPath,Tp,Start,ErrCtrl,Depend,obj,pwd,DispName,Group,Tag
SERV-delete [-stop-] [?RetVNm] SvrName  //include stopping if no -stop-
Switch: -boot -system -auto -demand -disabled -delayed-auto
```

## Parameters

- **specified SrvName."!" means disabling the service,otherwise enabling.**

## Examples

```text
SERV FBWF or serv-create imdisk,system32\drivers\imdisk.sys,1,2,0,,,,ImDiskVirtualDiskDriver
```

## Remarks

  This command can be employed to enable the FBWF service(if installed) to enlarge the writing
  space of the system disk so that PE can be run from CDROM.
  The returned outcome of query:ErrorCode ServiceType CurrentState ControlsAccepted
  Win32ExitCode ServiceSpecificExitCode CheckPoint dwWaitHint dwProcessId dwServiceFlags

