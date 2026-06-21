# RAMD

To set the size of RamDisk and return the outcome to a variable.[See ImdiskHelp.txt]
LoadPoint:driver letter or directory

## Syntax

```text
RAMD [P percentage of memory] [LMini] [HMax] [FSurplus]                       <===old format
RAMD <DrvName>[,DiskSize][,DiskFormat][,DrvLetter][,DiskVol][,CompMark][,Opt] <===new format
RAMD ImDisk*[VarName] [*[Offset]*Size*[Ctrl]*Devi] [--[f]ins [-rmeu-]] -a -t Type -m LodPint
     [-n][-o op1[,op2 ...]] [-f|-F SrcFileDeviName][-s Size] [-b Offset] [-v PartID]
     [-S SectSz] [-u ImDiskID] [-x 1 TrackSectNum] [-y 1 CylinTrackNum] [-p "format Param"]
RAMD ImDisk*[VarName]  -e [-s Size] [-o opt1[,opt2 ...]] [-u ImDiskID | -m LoadPoint]
RAMD ImDisk*[VarName]  -d|-D [-u ImDiskID | -m LoadPoint]
RAMD ImDisk* -BD -m Driver [--remove] [--nbrd]               <===notify to add/remove driver
RAMD ImDisk*[VarName]  -l [--id] [-u ImDiskID | -m LoadPoint]      <===ImDisk full format
RAMD ImDisk* -gui                                                  <===ImDisk graphic mode
```

## Parameters

- **DrvName**: Ramdriv or ImDisk; The unit for both Max and Mini is MB.
- **DiskSize**: The unit is MB,defined with Pxx Lyy Hzz,xx:percentage of available memory,
  yy:minimum,zz:maximum;
- **DiskFormat**: supportive of FAT,FAT32 and NTFS.If set,the disk will be formatted according to
  the params; If absent,not formatted.
- **DrvLetter**: the drive letter assinged to ImDisk.If the driver of Ramdriv is adopted,it
  should be absent together with "DiskVol";
- **DiskVol**: the volume to set when the disk is formatted with the driver of ImDisk.
- **CompMark**: 1:NTFS compression; 0 or null:no compression.
- **Opt**: 0x1 bit:Being 1 indicates physical memory mode,only intended for Imdisk.
- **Ctrl**: 0x1 bit:writable.0x10:self-unload upon shutdown.0x20:all self-unload upon shutdown.
  0x40:to disable self-unload. 0x100/0x200:with/without many w-mount for this disk
  --nbrd:not to broadcast the drive letter in ImDisk full format.

## Examples

```text
RAMD P20 L32 H64
RAMD ImDisk,L64,NTFS,B:,ImDisk1[,1,1] or RAMD Ramdriv,P20L32H64
RAMD ImDisk*ret*2097280s*5734529s*0*\\.\PhysicalDrive0  -a  -o hd -m Z:     //sector mapping
```

## Remarks

  The three params can be used separately.Eg,when RAMD P10 is used,10% of the memory will be
  set as RamDisk.
- When Ramdriv is used,the disk will be assigned a drive letter and formatted by the driver
  of Ramdriv and When ImDisk is used,by PECMD;
- When the driver for ImDisk is used,this command can be employed to expand a specified
  virtual disk like RAMD ImDisk,L128,NTFS,B:,ImDisk2.
- Pxx Lyy Hzz can be used separately.The minimum limited by PECMD is 1M and the maximum is
  (2GB for OSx86,32GB for OSx64);
- Mount WIM in the physical memory mode(RAMD ImDisk,L64,NTFS,B:,ImDisk1,,1).For FAT32,the
  size of the disk cannot be too small;
- If the command is executed successfully,the environment variable of the process
  %CurRamDisk% will return the drive letter,and it can be set as a system one with the
  command of ENVI as you need.
- Inner IMDISK needs cabinet.DLL.

