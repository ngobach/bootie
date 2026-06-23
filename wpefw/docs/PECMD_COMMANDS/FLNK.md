# FLNK

To establish a hard link or symbolic link of NTFS file system.

## Syntax

```text
FLNK <DestPathName>,<SrcFilePathName>[,LinkTypeMark]
```

## Parameters

- **DestPathName**: the name of file and directory to establish a hard link or symbolic link;
- **SrcFileName**: the path name of an existing file;
- **LinkTypeMark**: Num,if absent,0 by default indicating establishing a hard link,while non-zero
  symbolic link.

## Examples

```text
FLNK %WinDir%\System32\abcd.DLL,%CurDir%\abcd.DLL or FLNK %WinDir%\MyTool,%CurDir%\MyTool,1
```

## Remarks

- XP and 2003 OS do not support symbolic link or directory link,as well as trans-volume hard
  link;
- This command supports environment variable interpretation(example),as well as using
  wildcards;
- This command works by calling CreateHardLinkW of Kernel32.DLL or CreateSymbolicLinkW.
  Whether it succeeds or not has nothing to do with the system.
- -j or Type 2 indicates directory link.If SrcFilePathName is null,the link will be deleted.

