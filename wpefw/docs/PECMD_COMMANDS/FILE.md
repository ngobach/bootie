# FILE

To delete,copy,move or rename a file or directory.-m:multiple files,spaced by "; ",-:to end.
■-simpleprogress:to display progress bar,-progress:to display progress bar and file name,
  --file:to only operate a file,-force[q]:to delete by force(as possible)[itself first],
  -q:no warning(like IE temporary directory),-delme:to delete file or directory locked by
  itself.  -rd:rmdir  -su:super-right   -delay:delay

## Syntax

```text
FILE [-simpleprogress|-progress -force[q] -file -m -q] [-] FilePath[<operator>DestPath]
FILE FilePath>>NewName without parent directory  //to imitate the command of Ren in CMD,not
change the directory containing the file but change its name.
```

## Parameters

- **FilePath**: the name of the source file and directory.DOS wildcard "?" or "*" is applicable
  to deleting,copying and moving the file and directory;
- **Operator**: "->","=>",">>" refer to moving,copying,renaming the file and directory.
  Operators being absent indicates the operation of deleting. When renaming with ">>",be
  sure there are no DOS wildcards in FilePath and NewName.Moreover,there should be only one
  FilePath and NewName and NewName is sure not to have existed with its directory unchanged.
  Actually,"->" is also capable of doing this and seems more powerful than this.
- **DestPath**: the destination path to move or copy the source file and directory to.It should
  be a definitive name of file and directory without any DOS wildcard.

## Examples

```text
FILE %SystemRoot%\INF\*.INF=>%TEMP% or FILE pphk->pphk22 or FILE -delme %CurDir%
```

## Remarks

- If the system is started with RAMDISK,some useless files (like NTOSKRNL.EXE of 2M)can be
  deleted after booting to add to the writeable space for RAMDISK;
- When moving and copying,if only a directory is specified,the specified directory will be
  copied or moved to the destination only,as is shown in example 2; When operating a file,no
  operation will be performed if no specified directory exists in the specified destination,
  as is shown in example 3.
- The operation will be terminated if the specified destination directory or file does not
  exist,like "pphk" and "top.bmp" in example 2 and 3,if not exist;
- To move a folder to a new one,please refer to the example: FILE pphk=>pphk22
- Except renaming,the source and the destination can be multiple,separated by "; ".
  If there are multiple destinations,the number of sources should be the same as that of
  destinations,ie,they should be operated correspondantly; The destination should be the
  name of itself instead of the directory containing it.
  eg:FILE --m x.log;x3.log=>FOLDER\x11.log;HELP\x33.log
- -file Supportive: \\?\Harddisk0Partition2\A.B  \\?\HarddiskVolume5\A.B  \\?\ImDisk0\A.B

