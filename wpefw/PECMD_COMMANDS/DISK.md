# DISK

To get information of hardisk,unmount or assign drive letter and disk volume.

## Syntax

```text
DISK  [-check] [-skiptp:tp1;tp2] [-skippt:hd1:lpt1;hd1:lpt2]  [-from:DrvChar[Tbl]] [-cdrom]
      [VarName],[HardDiskID],[PartID],FuncID,[UDiskDrv[Tbl]] [,Option]
```

## Parameters

- **VarName**: string,can be a drive letter,the name of windows system directory beginning with
  '\',variable name in string or null;
- **HardDiskID**: number,the sequence number of a hard disk basing 0,can be absent;
- **PartID**: number,the sequence number of a partition basing 1,can be absent;
- **FuncID**: number,0 by default if absent;
- **UDiskVol**: a letter or driver letter,can be absent.

## Examples

```text
DISK ,,,1,U: or DISK ,,,2,U: or DISK uDisk,,, or DISK \Windows|\WinXP|\WinNT|\Windows7,,,1,U
DISK uAllPart,,1 or DISK Part,0 or DISK ,1,3 or DISK E:,1,4,1 or DISK Vol,0,2 or DISK ,,,3,U
DISK -check -from:C-V ,,,1,?:,0x71 //to order existing drive letters
```

## Remarks

- If "HardDiskID","PartID" and "VarName" are null,"FuncID" shall not.Bit 0 of "Option" means
  only sorting out the partition with a drive letter,Bit 1 verifying the validity of
  partition,bit 2 skipping patition of 0xEE/0xEF type,bit 4 sorting out hidden partition
  with a drive letter,bit 4 sorting out CDROM as well,and bit 5 limiting driver table.
  The command will assign drive letters according to following "FuncID":

## Sub-specification: 1

DISK  ,,,FuncID[,UDiskDrv][,Option]

## Sub-specification: 1.1

DISK  ,,,1 or 4[,UDiskDrv][,Option]   //4 means bit 2 of Option is set
  1.To reassign drive letters to all the visible partitions according to the sequence
    numbers of the fixed hard disk and partions,example 1;
    UDiskDrv=?, means the fixed hard disk follows.

## Sub-specification: 1.2

DISK  ,,,2 or 22[,UDiskDrv]  //22 is the first main partition;
  2.To assign drive letters to the first main partition of hard disks and then to other
    partitions,example 2;
    UDiskDrv=?, means the fixed hard disk follows.

## Sub-specification: 1.3

DISK  ,,,3,UDiskDrv[,Option]
  3.Not to ajust drive letters of fixed (and removable) hard disks,but the ones of U disk,
    example 10;

## Sub-specification: 2

DISK  VarName[,,,FuncID][,UDiskDrv]
☼If "HardDiskID" and "PartID" are both null,but "VarName" not,the command will perform
  respective functions according to the following two forms of "VarName":

## Sub-specification: 2.1

DISK  VarName
  1.If "VarName" is in the form of a string variable,the command will assign the number of
    hard disks of the computer to the specified "VarName",example 3;

## Sub-specification: 2.2

DISK  \system directory name[,,,FuncName][,UDiskDrv]
  2.If "VarName" is the system directory name beginning with '\',the command will seek and
    load the system registry and assign drive letters according to the assignment system of
    the hard disk.To seek more than one possible system directory names,use '|' to separate
    them.(example 4); If this fails,the command will continue with the subsequent plan of
    drive letter assignment according to the value of "FuncID",example 1,2 and 10;

## Sub-specification: 3

DISK VarName,,PartID
☼If "HardDiskID" is null,but "PartID" not,the command will assign the number of mounted
  partitions of hard disk of the computer to a specified "VarName",example 5;

## Sub-specification: 4

DISK  VarName,HardDiskID
☼If "HardDiskID" is not null,but "PartID" is,the command will assign the number of mounted
  partitions of the specified hard disk to a specified "VarName",example 6;

## Sub-specification: 5

DISK [VarName],HardDiskID,PartID
☼If "HardDiskID" is not null,nor is "PartID",the command will dispose in the following
  three ways according to the format of "VarName":

## Sub-specification: 5.1

DISK ,HardDiskID,PartID
  1.If "VarName" is null,the assigned drive letter and volume of this partition will be
    unmounted,example 7;

## Sub-specification: 5.2

DISK  drive letter or volume,HardDiskID,PartID,[FuncID]
  2.If "VarName" is a drive letter or volume(like E: or E:\),the command will assign a
    specified drive letter or volume to the partition.If "VarName" is absent or 0,or the
    partition is a hidden one,the command will not assign any drive letter or volume.
    If "FuncID" is not 0,the command will assign a drive letter or volume whether the
    partition is hidden or not,example 8;

## Sub-specification: 5.3

DISK  VarName,HardDiskID,PartID
  3.If "VarName" is not null or in accordance with the format of drive letter,the command
    will assign the drive letter of the partition to a specified "VarName",example 9;
☼"UDiskVol" indicates the initial drive letter assigned to the USB Flash Disk when
  the drive letters are being automatically sorted out.If absent,the drive letters of the u
  disk will not be sorted out;
☼The command for sorting out drive letters should be placed at the front of PECMD.INI.

