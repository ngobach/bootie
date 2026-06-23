# FDRV

To return the drive letter for the partition containing the specified "FileDirName"
(end with ":",without "\" at the end),or all drive letters of the system.

## Syntax

```text
FDRV <VarName><=>[FileDirName]
FDRV <VarName>=   //return all drive letters of the system in the form of C:|D:|E:|F:|...
FDRV <VarName>=*: //return all drive letters of the system in the form of C: D: E: F: ...
FDRV <VarName>=*  //return all drive letters of the system in the form of C D E F ...
FDRV <VarName>=?  //return all MS-DOS device names of the system
FDRV -link? Ret,AllName,LastName=SymName    //returns the linked object.
FDRV -vol [VolName][,FSName][,IDName][,MaxLenVarName][,MarkVarName][,UUIDName]=DrvName //
  return the volume,etc.
//-ab:not to search for AB floppy; -rsort:in reverse; -idle:idle
drive letters; -idlec:to exclude AB.
FDRV -setvol DrvName=Vol    //set the volume.
```

## Parameters

- **VarName**: to store the drive letter for a partition;
- **FileDirName**: valid file or directory name(can be absent.Refer to the explanation in REM for
  other implication of being absent.

## Examples

```text
FDRV fDrive=C:\a or FDRV Drvs= or FDRV *vol V=\\?\Harddisk0Partition2 or \\?\HarddiskVolume5
or\\?\ImDisk0\ or\\?\Volume{cc0ac7c9-27c4-11e5-ba3b-806e6f6e6963}\ or\\?\STORAGE#Volume#{cc0
ac7c0-27c4-11e5-ba3b-806e6f6e6963}#0000008481CD9800#{53f5630d-b6bf-11d0-94f2-00a0c91efb8b}\
```

## Remarks

- If FileName is a relative path,the returned outcome is the working directory of PECMD.EXE
  or config file;
- If "FileDirName" is absent,all drive letters of the system will be returned in the form of
  C:|D:|E:|F:|...;
- The outcome of example 1 is "C:" and example 2 is a list of all drive letters of the
  system(The list is not settled but changeable all the time).

