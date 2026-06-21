# PUTF

To modify the data of a specified file in BINARY.The suffix # means source is a PE Var, with
  two extra bytes of zero permitted to be written in at most.

## Syntax

```text
PUTF[*][.][#] [-16] FileName,[StartPos[#LenLmt]],<DataQue|PEVarName|#[.]InternlDataID[|Dir]>
PUTF -dd [-bs=BS] [-ibs=BS] [-obs=BS] [-count=COUNT] [-len=LEN] [-skip=SKIP] [-skipb=SKIPB]
    [-conv=notrunc] [-quick|-qspace] [-va] <FileName>,[ExportStartPos],InFileNameOrInt64
```

## Parameters

- **FileName**: the existing file name,supporting environment variables interpretation; -16:data
  queque in hex.    -v: Write PE_VAR
- **StartPos**: number,a starting position relative to the base address of the file,hex supported
  ,like 0x100; It starts from default 0 and can be followed by the suffix TGMKS.
  #LenLmt being absent indicates all data.
- **DataQue**: [*ReptFactor] number,the data to write in from the starting position.Numbers are
  separated by space,like *1M 0x00 0x23 0xAD *1K 0x1C 128.The internal data ID of the
  resource lies beneath "EXEDATA"; "." indicates not automatically extracting.
  Format 2 refers to the mode of high speed binary block delivery.BS:the number of bytes in a
  block.COUNT:the number of blocks.SKIP:the starting number of blocks of the file to input.
  SKIPB:the starting number of bytes of the file to input which can be followed by TGMKS.
  FileName is permitted not to be in existence.-conv=notrunc:not to shorten the existing file
  The file name to input can be zero,which indicates 0 clearing.
  -quick:sparse file.-qspace:to allocate space quickly. -va:not initiated space.
  Logical drive A:-Z:, CDROM \\.\CDROM0~N, tape \\.\TAPE0~N, disk \\.\PhysicalDrive0~N,
  NUL:a dummy file.LEN:length in number of bytes,prior to COUNT.
  -fd=HNNDLE: accelerate output

## Examples

```text
PUTF %CurDir%\Memu.DAT,0x400,0x23 0xAD 0x1C 0x80 0x6F or PUTF* A.DAT,,#100
```

## Remarks

- The command of PUTF modifies the starting position of the file relative to its base
  address in a way of overwriting without changing the length of the file,but the length of
  the new file will not be limited.New file as it is,if the specified starting position is >
  1 or far beyond the end of the file,a hole will be left in between(It does not occupy any
  space in some advanced file system like NTFS,nor does it have any bad influence); PUTF*
  can expand the file.The suffix "." indicates directly writing into a text(ANSI).
- The corresponding command to read a file in BINARY is GETF.LenLmt is only applicable to PE
  variable name and internal data ID.
- The command to read or modify a file in string is READ and WRIT.
- The resource can be indicated as:"[FileName]""#RescName[|DirName]".The couple of "" behind
  can be absent.RescName can be non number characters at this moment.
- Supportive: \\?\Harddisk0Partition2\A.TXT   \\?\HarddiskVolume5\A.TXT  \\?\ImDisk0\A.TXT
- FileName leading with & is PE_VAR.

