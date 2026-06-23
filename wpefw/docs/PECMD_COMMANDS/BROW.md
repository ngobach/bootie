# BROW

To browse file or directory on the disk.-fix:to crack down the shield of system directory
(not sure to succeed).

## Syntax

```text
BROW [-fix] <VarName>,[[*|&]InitPath],[Tips],[ExtName],[AddMark]
```

## Parameters

- **VarName**: to store the result of browsing.
- **InitPath**: the file or directory selected by default in the browsing window.If absent,the
  system default path will be positioned; "*" indicates a dialogbox for directory browsing.
  "&" a dialogbox for file saving; If the leading character is absent,the default is a
  dialogbox for file opening.
- **Tips**: can be absent.If so,tips will be the system default ones.
- **ExtName**: to browse the file with the specified extended name.Being absent indicates all.
  It can also be a string of many options:[TIPS1|*.EXT1|TIPS2|*.EXT2|],like:
  INI FILE|*.INI|All|*.*|
- **AddMark**: style of the dilogbox for directory browsing:"no [Editbox],with [NewDir]button,
  no FileDisplay" by default.
  0x10:with Editbox; 0x200:no NewDir Button; 0x4000:FileDirCompoundSelect
- **SelFile**: 
  0x200:multiselect; 0x80000:browser style; 0x2000:warning of NEW; 0x02000000:
  to exclude shortcuts of recent visit; 0x800000:size changeable; 0x1000:file must be in
  existence; 0x40000:short file name; 0x2:warning of overwriting; 0x1:read-only if checked;
  0x800: warning of invalid path; 0x8000:to exlude read-only files; 0x4:to remove checkbox
  for read-only; 0x10000000:to select read-only or hidden files

## Examples

```text
BROW Boot_Ini,C:\BOOT.INI,Please select a file,INI or BROW Tag,*C:\Windows,Please select a
directory
```

## Remarks

- The hex sum of the three marks "with Editbox,no NewDir Button,FileDisplay" in the
  dialogbox for directory browsing is 0x4210.
- This command will open a dialogbox for users to browse files or directories and select one
  in it.
- The selection can be stored in an environment variable of a process.Example 2 can be
  referenced via the environment variable %Tag%.
- BROW is often executed after the command of INIT or on the desktop.
- This version of PECMD differs from those of old version,BROW no longer affects current
  working directory and thus it will not hang on in the target directory any more.

