# UPNP

To execute the functions of BartPE.EXE

## Syntax

```text
UPNP [$]<Params>
```

## Parameters

- **"$" indicates displaying the executive interface of BartPE.EXE;**
- **Command line params of BartPE.EXE.**

## Examples

```text
UPNP -pnp or UPNP $-pnp
```

## Remarks

- This command is embedded with the executalbe codes of the new version of BartPE.EXE,and
  thus WinPE can fulfill the functions of BartPE.EXE without the file of BartPE.EXE;
- This command only supports PE of NT5.x series and is executed in the blocking mode,ie the
  next command will not be executed until the preceding one is finished.Pay attention to the
  params and their case sensitiveness.

