# HOME

To set and lock the IE homepage and disable the registry editor.

## Syntax

```text
HOME [IEHomePage],[Num1],[Num2]
```

## Parameters

- **IEHomePage**: support all kinds of websites and local webpage files,like"http:\\www.mssb.com"
  If it is a local webpage file,a full name of the file is demanded(environment variable can
  be contained),like "%SystemRoot%\ReadME.htm".
- **Num1**: If not 0,the IE homepage will be locked,ie,no modification to it.If absent,it is 0 by
  default,ie,modification is permitted.
- **Num2**: If not 0,the registry editor will be disabled.If absent,it is 0 by default,ie,enabled

## Examples

```text
HOME http:\\www.sbms.com,1,1 or HOME %SystemRoot%\ReadME.htm
```

## Remarks

- This command can be used in a script and command line; If teh IE homepage or registry
  editor is locked by some evil tools,it can be employed to unlock.
- Example 1 indicates setting "http://www.sbms.com" as the homepage and disabling modifying
  it in the meanwhile,as well as forbidding the use of the registry editor.

