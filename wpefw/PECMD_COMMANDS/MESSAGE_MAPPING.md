# MESSAGE MAPPING

## Parameters

- **ENVI @WinAndCtrlName.MSG=[_$*+]MsgID**: [:[wPrm ParamName],[lPrm ParamName],] command to
  execute  //to define the capture of message mapping.
  front command by default; _ refers to rear command; $ alternative command,ignoring system
  response and meanwhile return outcome code; * used in the command of TRAP B. +:super mode
  "Command to execute" beginning with - indicates executing from the window where it lies
  (execution stacks). <:DisLogsCmd
- **ENVI @WinAndCtrlName.POSTMSG=[**: VarName;]MsgID[;Param wPrm[;Param lPrm]] //Send msg asynch
- **ENVI @WinAndCtrlName.SENDMSG=[**: VarName;]MsgID[;Param wPrm[;Param lPrm]] //Send msg synch
- **ENVI @@POSTMSG=[**: VarName;]WinID;MsgID[;Param wPrm[;Param lPrm]]         //Send msg asynch
- **ENVI @@SENDMSG=[**: VarName;]WinID;MsgID[;Param wPrm[;Param lPrm]]         //Send msg synch
  The two WinIDs above can be *[?Timeout[?Interval]][*WinClassName*][Title]
- **Applicable to _SUB window and control.For MsgID,see**: WM_MSG.WCS,IMPORT is applicable.
  The VarName returns the outcome(Its meaning depends on the message).
- **MsgID beginning with # indicates PECMD will apply user-defined message 1-N. wPrm,lPrm are**
  param names,which can be referenced._ after = indicates entering next-half-response mode,
  ie,to respond after the system response(to respond to derived messages in reverse order).
  ";" above can be replaced with ",".Params for message can be numbers,@PEVarName(ENVI$# to
  fill non-string objects),$String(only applicable to SENDMSG).
  .MSG=MsgID can be:<COMMAND|NOTIFY>#[CtrlID][# minor MsgID]

## Examples

```text
ENVI @Win1.MSG=#1::wp,lp,MESS- HELLO wp=[%wp%],lp=[%lp%]@#OK
 ENVI @Win1.POSTMSG=#1,1,2             //to trigger message #1.
 ENVI @Win1.MSG=%&WM_LBUTTONUP%:MESS You have clicked.
 ENVI @Edit1.MSG=%&WM_MOUSEENTER%:ENVI @Edit1=The mouse has entered.
 ENVI @@SENDMSG=%__WinID%:#2;@str1;@str2 or ENVI @@SENDMSG=%__WinID%:#2;$Line11;$abc11
```

## Remarks

- EXEC* @CtrlName=CMD:The outcome of the "CMD" can be displayed synchronously on  controls
  like EDIT/MEMO/LABE.                                                       Refer to EXEC.

## Model / Use Cases

How PECMD captures the echo of CMD command line(not via temporary file)? Eg:how to capture
the echo of CMD command "dir d: /d"?
 EXEC*  &V=!cmd.exe /c dir d: /d
 MESS. [%&V%]@DEBUG#OK

