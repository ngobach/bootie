# EJEC

To eject or close the specified CDROM drive tray; To romove the specified USB drive.

## Syntax

```text
EJEC [C-|U-|X:|DiskNum],[OPID]
```

## Parameters

- **Param "C-"**: to operate all possible CDROM drive trays;
- **Param "U-"**: to remove all possible USB disks;
- **Param "X**: ":to operate CDROM drive or USB disk with the specified drive letter;
- **OPID**: to specify the operation of ejecting or closing CDROM drive tray.Absent,0 by default,
  indicating ejection,1:to close,R:to remove the USB disk.

## Examples

```text
EJEC C- or EJEC U- or EJEC H: or EJEC H:,1 or EJEC C-,1
```

## Remarks

- This command is applied to a simplified PE which does not have the icon for u disk
  management in its system tray; For PE which has it,it is advised to use the system tray.
- Do not use this command in a config file; The command of INIT with the param "I" will
  install the function of this command to the menu of system tray.

