# DFMT

To call FORMAT.COM to format disk.

## Syntax

```text
DFMT <DrvLetter>,<DiskFormat>[,DiskVol][,QFDisk][,UnitSize][,CMPS]
DFMT* FORMAT params
```

## Parameters

- **DrvLetter**: to specify a disk to format;
- **DiskFormat**: supportive of FAT,FAT32,NTFS and exFAT.If params are set,format according to
  them.If not,format skipped;
- **DiskVol**: Volume to set when formating disk;
- **QFDisk**: number,not 0:quick format; 0:to format by default; If absent,quick format;
- **UnitSize**: number,absent or 0 indicates default,not 0 indicates values as 512*(2^n),ie,512,
  1024,2048,...,65536.
- **CMPS**: number,absent or 0:not to compress,1:to compress.It is only applicable to NTFS.

## Examples

```text
DFMT E:,FAT32 or DFMT R:,NTFS,RamDisk,1
```

## Remarks

- This command supports formatting hard disk and virtual hard disk,but not floppy disk;
- Unit size is recommended to be default 0 or simply absent;
- This command is executed in silent mode,displaying no information.So be cautious when
  operating.The format is in a complete form.

