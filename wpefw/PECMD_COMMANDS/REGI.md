# REGI

To read,set or delete the data in registry.  --name DataVal is VarName
--k:only to create a key  --byte:byte stream  --v[-]:Not Save(-Not Auto.)  --qk:quick
--init: Get empty string on fail   --a[v|k] List all[Val|SubKey] -su:super-right

## Syntax

```text
REGI [--a[v|k]] [LeadChr]<HKLM|HKCU|HKCR|HKU|HKCC><\SubItem\\><KName,VName>[,TypeVName]
REGI [--name] [--0] [--16] [LeadChr]<HKLM|HKCU|HKCR|HKU|HKCC><\SubItem\\>[[KName][Operator]
     [[TypeChr]DataVal]]
```

## Parameters

- **Leading characters  "$","+","#","@","*" or "**" or "*$","~" or "~~" indicate reading**
  resepctively the data of REG_SZ,REG_DWORD,REG_BINARY,REG_MULTI_SZ and REG_EXPAND_SZ in the
  registry.They indicate types of data when writing into the registry.
  ~~ indicates reinterpretating the environment variables in the data of the registry.
  ** indicates the substring "\0" will be interpreted as "\\0" when reading and *$ refers to
  a multiline string,but when writing,the substring "\\0" will be saved as "\0" and *$
  indicates saving the multiline string as parts separated by "\0",a separate line for each.
  When reading,there are 2 types:.indicates the arbitrary type and can be followed by
  TypeVarName; ? refers to the type of query(NI refers to the unset null value).
  "^","+","b","u","n" refer to the type of REG_LINK,REG_QWORD,REG_QWORD_BIG_ENDIAN,
  REG_MUI_SZ,REG_NONE,intelligently preposed.--t:numeric type,to directly specify any other
  type of data,which is the same as REG_BINARY.
- **SubItemName**: the full name of the registry entry beneath the selected ROOTKEY,also such
  form is directly adopted as HKEY_LOCAL_MACHINE.
- **KeyName**: the name of the key to operate.If absent,the default key will be operated.If the
  operator is "!" without "=",the whole subitem will be deleted.
- **VarName**: to store the returned value.If there is a comma but no VarName,a default name
  RegDat will be adopted.No comma indicates setting or deleting the data of the registry.
- **operator**: The operator "!" indicates deleting the whole subitem; The operator "=" without
  data indicates deleting; The operator "=" with data indicates setting data.
- **TypeChar**: the type of data.Being absent indicates the type of string."#" refers to the type
  of REG_DWORD,like "#0x20"; "@":REG_BINARY,like "@23 34 90 255"."*":REG_MULTI_SZ,like
  "*string1\0str2\0\0"."~":REG_EXPAND_SZ,like "~%SystemRoot%\System32".It is strongly
  suggested using the leading characters to indicate the type of data to avoid being
  confused with a string.Priority is given to the leading characters.
- **DataVal**: Null value of a string is indicated as "".The type of REG_DWORD and REG_BINARY is
  supportive of hex numbers.--16 coordinating with @# indicates hex.--0 coordinating with =
  indicates creating a null item and coordinating with \! indicates clearing the subitems,
  values and default values beneath it; The DataVal can be divided into several subitems
  --0:number N,1:to clear the default value,2:to delete all subitems,4:to delete all values.
  multiple --0:N is permitted and the numbers can be added up,which means subitems are
  combined(like 1+4=5,1+2+4=7,ie,5 indicates the combination of subitem 1 and 4,while 7 of
  subitem 1,2 and 4).

## Examples

```text
REGI HKCU\SOFTWARE\PECMD\Version=#1200 , REGI $HKCR\lnkfile\IsShortcut=
REGI $HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders\Cache,IECache
TEAM SET Val=" cdcsd ,| SET PT=Software\"a ,| REGI $HKLM\%&PT%\\%&Val%=1\\2 ," //Super Mode
```

## Remarks

- This command is complicated,please refer to the instructions carefully.Example 3 shows
  reading the position of IE cache.
  To delete> REGI HKCU\abc=  or TEAM REGI HKCU\abc=|
  Null string> REGI HKCU\abc="" or TEAM REGI HKCU\abc= | or REGI $HKCU\abc= //or other leading
  characters for string (* ** ~ ~~).
  or REGI HKCU\abc=%&NOTHING%  //%&NOTHING% as a variable of null.
  If "" itself,the leading characters themselves or a purely null string is needed to be
  input,it can be transfered to a variable or a leading character can be employed to achieve
  this.
  To input null characters at front,format 1> REGI $HKCU\abc=    123
  To input null characters both at front and back,format 2> TEAM REGI $HKCU\abc=    123    |
  To input null characters both at front and back,format 3> REGI $HKCU\abc=%VAL%   // %VAL%
  contains any combination of null characters.
  REGI --0 $HKCU\abc\! //to clear; REGI --0 $HKCU\abc= //to create a null item;
  REGI --16 @HKCU\abc=313233 //hex

