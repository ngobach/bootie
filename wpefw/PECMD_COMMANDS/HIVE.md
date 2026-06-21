# HIVE

To load or unload registry config file,euqual to the functions of LOAD and UNLOAD of REG.EXE

## Syntax

```text
HIVE [-u] [-super[_r]] [-quick] [*] <SubKeyName>,[FileName]
```

## Parameters

- **-super_r**: to set the authority of child keys by force. -quick:not add authority(default);
  -u:to load to HKEY_UESRS.
- **SubKeyName**: the subkey name in HKEY_LOCAL_MACHINE when loading or unloading the specified
  registry config file.* indicates being exported to a file.
- **FileName**: the name of the registry config file to load.If absent,the command will unload
  the subkey with the specified SubKeyName.

## Examples

```text
HIVE PE,F:\BOOT\PE or HIVE PE or HIVE PE,%CurDrv%\BOOT\PE
```

## Remarks

- After HIVE loads the registry config unit,PECMD will set the account Everyone as
  acquiring permissions of full control so that users have the privilege to read and modify.

