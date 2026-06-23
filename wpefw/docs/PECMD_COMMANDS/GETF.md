# GETF

To read the data of a specified file in BINARY and assign it to a specified variable.
The suffix # indicates reading the binary PE object.

## Syntax

```text
GETF[#] [-err:errcode] [-g=ColNum[@SpacedStr]] FileName,PosLen,VarName
GETF -find LargeFileName,[LargePos[#LargeLen[#SubLen[#SubOffset[#BlkSZ[#OffsetInBlk]]]]]],
    [*[<times>]]VarName,DestFile|*DestVarName            //to search in binary *:list
```

## Parameters

- **-g=ColNum**: to automatically insert a line break in the position after a certain number of
  columns.   -fd=<HNNDLE|?NameHandle>: accelerate -free=HANDLE:free
- **FileName**: a valid name of file,supporting interpreting environment variables; It can be a
  physical drive like \\.\PhysicalDrive0,a logical drive like C: and a partition device like
  \Device\Harddisk0\Partition1; It can also be #[.]InternalSrcID[|Dir],.indicates raw data.
  The directory for special source can be indicated as #Num. -bin or begin with &:PEVAR
- **PosLen**: number,consisting of the starting position and the length (byte) to read,separated
  by "#",like 1023#32; The suffix TGMKS can be present.K=1024,S=512.Length being * indicates
  reading to the end of the file.
- **VarName**: string,to store the outcome of reading.
- **-find**: indicates the starting position of a binary search target in a large file.LargePos
  refers to the start,LargeLen indicates the limit and SubLen indicates the target length
  limit.If LargePos,LargeLen,SubLen and SubOffset are absent or <=0,they will be ignored;
  BlkSZ is 512 by default.LargePos%BlkSZ can be used as the OffsetInBlk.

## Examples

```text
GETF %CurDir%\Memu.LST,0x400#0x10,FileData or GETF %CurDir%\Memu.LST,-1,FileLength
GETF -bin &Src,2#4%,Dst or GETF#  %CurDir%\Memu.LST,0x400#0x10,&DataBin
TEAM ENVI$# &DATA=33 33 | GETF -find A.ISO,0#1M#2##512,&POS,*&DATA //512Bytes-group find
TEAM ENVI$# &DATA=33 33 | GETF -find A.ISO,0#1M#2##1,&POS,*&DATA   //1Byte-group find
```

## Remarks

- The starting position that GETF reads is relative to the starting address of the file.To
  read the first byte of the file,the starting position shoulb be 0x00;
- The outcome of the command is indicated with hex numbers,separated by space between them.
  If the starting position or length is absent,both will be set as 1 by default;
- The corresponding command to modify files in BINARY is PUTF.It is not suggested treating
  super large files with the commands of GETF and PUTF;
- If the starting position is <0 or exceeds the length of the file,the outcome of the
  command will be the length of the file indicated in decimal and the error outcome will be
  0xFFFFFFFF by default;
- Besides,the commands to read or modify files in string are the commands of READ and WRIT.
  For large mounts of data(7K),PE variables should be employed;
- Data format:Numbers are spaced,like 0x23 0xAD 0x1C 0x80.
- -find:BlkSZ refers to the aligned size,512 by default.If it is set as 1,the command will
  search all the content in the way of byte alignment. Times<0 is reverse.
- The resource can be written as "[FileName]""#SrcName[|DirName]",and the pair of "" behind
  can be absent.The SrcName can be non-figure characters at this time.
- To support UD files: ud:UD FileName:UD InternalFileName
- Supportive: \\?\Harddisk0Partition2\A.TXT   \\?\HarddiskVolume5\A.TXT  \\?\ImDisk0\A.TXT

