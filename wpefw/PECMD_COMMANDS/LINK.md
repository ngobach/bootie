# LINK

## Syntax

```text
LINK[AltDelimiter] [--f] [-][_|^|!]<ShortcutPath><,DestPath>[,ExecParam][,IconPath#IconIdx][
                                             ,Rem] [,StartPos][,HK]
LINK[AltDelimiter] [--f] [-][_|^|!]<ShortcutPath><,DestPath>[,ExecParam][,IconPath][,IconIdx
                                            ][,Rem][,StartPos][,HK] <==Old style
LINK[AltDelimiter] ? [--f] ShortcutPath,[DestPathVarName][,ExecParamVarName][,IconPathVarNam
    [#[IconIdxVarName]]][,RemVarName][,StartPosVarNam][,HKVarNam][,DspMarkVarNam] //to query
To create a shortcut.  ^#~+- is AltDelimiter
```

## Parameters

- **ShortcutPath**: to specify a path for the shortcut to create without the extension ".LNK".
  --f:not to add ".LNK" automatically.
- **DestPath**: to specify the destination file or directory for the shortcut.If not exist,the
  shortcut will not be created.If this command is used in a command file,the DestPath can be
  a relative one(relative to the path containing the config file)
- **ExecParam**: the execution params for the target program.
- **IconPath**: the path of the shortcut icon.
- **IconIdx**: the index number in the file resources for the shortcut icon,0:the first icon;
  absent:default.
- **HK**: hotkey,the "#" or "," before IconIdx cannot be absent at this time.

## Examples

```text
LINK !%Desktop%\ADSLConnection,RASPPPOE.CMD,,RASDIAL.DLL#19
LINK ? %DeskTop%\ADSLConnection,&FN,&ARG,&FICON#&IICON,&COMMENT,&START,&KEY,&fSHOW
```

## Remarks

- "_" indicates starting the program in the way of being minimized and "!" hidden,both of
  which can be used to minimize and hide the window of the command."^" indicates being
  maximized."*" indicates creating a shortcut by force though DestPath does not exist.
  "," can be transfered to a variable.

