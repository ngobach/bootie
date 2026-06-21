# FORM

To return the media type of a specified drive.☼-raw:not to distinguish USB(to avoid floppy)
-x:floppy can be distinguished in details(2=1.44M).

## Syntax

```text
FORM [-raw] [-x] <[VarName][,BUSNAME]>=<*|[\\.\]VolName|DeviceName>
FORM <-free|-userfree|-userall>  [VarName=<VolName|Dir>
```

## Parameters

- **VarName**: to store the media type of a partition with a specified drive letter.BUSNAME
  stores the bus type to make up for the insufficient information of the media type.
- **VolName**: a valid drive letter or name of file and directory existing in the system.
  * indicates returning the list of drive letters.
- **DeviceName**: \\.\PhysicalDrive0,etc:to return the media type and bus type.
- **-free**: all free space(byte),-userfree:free space available for current user,-userall:all
  space for the current user.

## Examples

```text
FORM DriveType=F: or FORM DriveType=C:\Windows or FORM DriveType=%CurDir%
```

## Remarks

- VolName can reference environment variables,like example 3;
- The returned value of the command can be the following strings:DRIVE_UNKNOWN,
  DRIVE_NO_ROOT_DIR,DRIVE_REMOVABLE,DRIVE_FIXED,FUNCTION_ERROR,DRIVE_REMOTE,DRIVE_CDROM,
  DRIVE_CDROMUSB,DRIVE_RAMDISK,DRIVE_USBFLASH,DRIVE_USBDISK or numbers(others).
- This command,coordinating with FDRV,is capable of listing the media types of all drives in
  the system.                                                                 Refer to FDRV.
  bus type BusTypeUnknown=0,BusTypeScsi=1,BusTypeAtapi,BusTypeAta,BusType1394,BusTypeSsa,
  BusTypeFibre,BusTypeUsb,BusTypeRAID,BusTypeiScsi,BusTypeSas,BusTypeSata,BusTypeSd,
  BusTypeMmc,BusTypeVirtual,BusTypeFileBackedVirtual,BusTypeSpaces,BusTypeNvme,BusTypeSCM,
  BusTypeUfs,BusTypeMax,BusTypeMaxReserved=0x7F ,<0 error

