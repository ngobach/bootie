# PART

To create,delete,modify,hide and list partitions.

## Syntax

```text
PART [-gui][-usb][-fs][-up] [-admin] [-super] [-align[=..]] [-alignend[=..]] [-clear] [-raw]
   [-swap:id] [-CHS=C:H:S] [del/init] [-img=[*Off*SZ*]FileName|Disk]#Part [a|-a] type
   [[-]start [len]]
PART -gpt [-up] [-admin] [-super] [-align[=..]] [-alignend[=..]] [-clear] [-raw] [-cmp]
   [del/init] [Disk]#[+]Part  -|typeguid  -|start  -|len|@FinalSectorAddr guid -|Attr -|Name
PART update  [Disk]
PART [-ahup] hupdate[f]  [Disk]    //to flush the disk.     f:power mode(rebuild disk id)
PART [-devid[x]] list <disk|cdrom|floppy>  [Disk],VarName //-devid:to return disk DEVID;
     -devidx:to return physical ID.
PART [-hextp] [-phy] [-phy#] [-fill] list part       [Disk[#Part]],VarName
PART [-hextp] [-fill] [-raw] [-cdrom] list drv       DrvLetter,VarName
PART -raw list drv  DrvLetter,VarName     //DevNo. Part DrvType Bus Drv MideaType
PART [-drv] list volume [disk:Part|Drv],VarName //List:Volume Dev Part DevTp PartTp Off Len
PART [-cdrom|-floppy] list parent     <DevNo|DrvLetter>,VarName
PART [-cdrom] list dep   <DevNo|DreiverLetter>,VarName    //query source file
PART /mbr[=nt6|=win|=nt5|=dos|=[*LenLimit*]FileName]   [-img=FileName|Disk]
PART [-phy] /pbr[=nt6[fat]|=win[fat]|=nt5[fat]|=dos|=[*LenLimit*]FileName]
     [-img=FileName|Disk]#Part            //fat refer to the file systems of FAT/FAT32/ExFAT
```

## Parameters

  ѺIn the safe mode by default,this command will only operate the generally unused main
  partition 3 or 4 excluding the extended partitions.
  -gui:graphic mode; -usb:to list USB disk only; [-report[: RetVarName][ DiskNo]]to display or
  output the list, ignorant of other params. -mount-:not show label with no drive letter
  -admin:advanced mode(dangerous),capabile of operating partition 1 and 2;
  -super:super mode(rather dangerous):capabile of operating the extended partition;
  -align:start aligned,cylinder aligned by default,followed by =64K,=128K,=512K,=1M etc;
  -alignend:end aligned,-align by default,followed by =64K,=128K,=512K,=1M etc.
  -clear:to clear the valid information in the partition by force; -raw:not to make any
  ajustment to start/len,or to return (Dev# Part# Type Bus).
  -[x]up: After the disk is updated,the partition table will be updated to the OS,x to enhance
  -hup:to flush the removable disk; -ahup:to flush the disk; -iv=n:to extract the item n#.
  -fs: The "unformatted" message window will not pop up for a newly-created partition.
  C:H:S indicates HeadsPerCylinder:SectorsPerHead:SectorSize
  Disk:from hd0 to hdN,hd0 by default,the leading character hd can be absent for this type.
  Part:1~4.indicating the physical ID for the 4 main partition items in the partition table.
  It is settled and different from the logical ID used for mounting,which is not settled.
  If the partition table is null or it is an extended partition,there will be no ID for
  operating(creating)it; If the preceding partition becomes null,the ID number will be
  subtracted by one.If the position and length of the partition are different from the
  original ones,only after the partition is hidden or deleted can the partition item be
  modified.See appendix.
  +L:to add a logical partition at the end of the extended partition.
  -L:to delete the last logical partition of the extended partition.
  If the partition ID is the physical ID for the logical partition(>=5),it is practicable
  to modify its activation,type,start and length.
  [a|-a]: a indicates activation; -a:to remove the activation,otherwise it remains unchanged.
  type:The type of a partition,0~255.0:hidden totally,unrecognized by partition tools and OS.
  If the partition is assigned with a new type,the partition can be put in use right away
  .If <0 or absent,it remains unchanged. Disk,Part,type,start and len can be in hex.
  start:the starting position on the disk.The starting byte of the disk is 0.The disk capacity
  is the multiple of 512 bytes(automatically contracted and aligned internally).
  It can be 0.If <0 or absent,it remains unchanged.- indicates figuring out the end of
  the last partition using "list",and any ajustment to the alignment of the end will not
  have an effect on "len".For a logical partition,the suffix + can be added indicating
  skipping a certain length.
  len: It can be 0.If <0 or absent,it remains unchanged.Start_0/len can be followed by the
  unit suffix TGMKS.K:1024 bytes,S:512 bytes.
  del: to delete the partition thoroughly.
  The command to delete is: PART del [Disk]#Part or PART [Disk]#Part -a 0 0 0
  init:to initialize the GPT/MBR partition.
  fix:to fix GPT partition Table (checksum,flag).
  [x]update [-axup] [Disk]:to update the partition table to OS,x to enhance.Disk 0 by default.
  hupdate [-ahup] [-up] [Disk]:to hard-flush the partition table(to u disk).Disk 0 by default.
  list disk:to list all the partitions if Disk is absent.If present,to return such information
  of the disk as:DiskID,Size,CylinNum,HeadNum,SetorNum,SectorSize,MediaType,
  Sign(Hex),BusType,PartType,RemvFixDisk.
  list part:[Disk] hd0 by default,to list all the partitions if Part is absent(-:null,@:
  extended partition).If present,to return such information of this partition as:
  PartID,Type,BootMark,BaseOffset,Size,HideSectorNum,End,DrvLetter.
  -hextp:type of hex 0xNN. -phy:partition in physical ID,4 main partitions 1-4,and
  logical partitions 5-N.-phy#:to export physical ID.
  -fill:There is a placeholder * if no drive letter is assigned.
  The partition information of GPT is returned as follows:
  PartID,TypeGUID,GPT,BaseOffset,Size,Attr,PartGUID
  list drv: The drive letter in the outcome of this command is basically the same as the one
  of "list part",except that a disk ID is added at the end of its outcome.
  VarName:  to return the outcome of query.
  "MS Basic Data" partition by default,Attr:0.+ before "Part" indicates the insertion mode,the
  original partition is moved backwards once.Physical ID:1~128.
  -cmp:to compress the null item in the partition table(otherwise unrecognized by DG yet its
  use is not affected).
  Attr:
  0x1000000000000000:Read-only       0x2000000000000000:shadow  0x4000000000000000:hide
  0x8000000000000000:no drive letter 0x0000000000000001:dispensable partition in  PC
  /mbr:to rewrite MBR,nt6(win) by default.FileName can be specified.The file cannot be too
  large.FileName beginning with # indicates the resource number in EXEDATA.
  LenLimit refers to the utmost necessary length to read from the front of the file.The second
  * can be absent if there is no confusion,for example,if there exists #.
  /pbr:to rewrite PBR,nt6(win) by default.FileName can be specified.The file cannot be too
  large.FileName beginning with # indicates the resource number in EXEDATA.
  LenLimit refers to the utmost necessary length to read from the front of the file.The second
  * can be absent if there is no confusion,for example,if there exists #.

## Examples

```text
PART  1             //to fix the valid mark for the partition table in disk 1.
PART  1#3  a        //1#3 to activate the partition.
PART  1#3  -a       //1#3 to cancel the activation of the partition.
PART  1#3  0x17 1048576000  2097152000  //100M+200M partition 1#3,hidden NTFS.
PART  1#3  0x17     //to hide 1#3 partition.
PART  1#3  0x7      //unhide 1#3 partition.
PART  1#3  -a 0     //to hide completely,unrecognized by other partition tools or OS.
PART  1#3  -1 -1 1048576000 //to ajust to 100M,it is actually needed to contract the size of
                              the file system first or enlarge it afterwards.
          (eg:1.To ajust from 100M to 50M:1)contract the size of the file system first;
                                          2)contract the size of the partition using Part.
              2.To ajust from 50M to 100M:1)enlarge the size of the partition using Part;
                                          2)enlarge the size of the file system afterwards.)
PART  1#3  -a 0 0 0 //to delete the partition without influence its data,and remapping and
                      mounting is practicable.
PART  del  1#3      //to delete the partition,without influence its data,and remapping and
                      mounting is practicable.
PART  /MBR=*63s#5 0 //to write resource No.5 in EXEDATA into MBR of disk 0,63 sectors
                      at most.
PART  /MBR=*63s*%CurDir%\MBROS.MBR  1  //%CurDir%\MBROS.MBR to write into MBR of Disk 1,
                                         63 sectors at most.
PART  1#5 a  0x7    //to unhide and activate the logical partition 1#5.
PART  -super -force -clear -up -hup  1#2 -a 5 2.5G  3.5G //to create extended partitions.
      -clear:unrecoverable.
PART  -up 1#+L   0x7  +32256  57544704 //to create a logical partition at the end of the
                                                                       extended partition
PART  -up 1#-L      //to delete the last logical partition.
PART  -up -super -gpt -cmp 1#+1 {EBD0A0A2-B9E5-4433-87C0-68B6B72699C7}   1G   5G
      {BC65AB2C-B5AF-4197-92B2-00E808FDEE09}    0x9000000000000000  "Basic data partition"
PART  -up -super -gpt -cmp del  1#2  //to delete the GPT partition without influence its
                                       data and remapping and mounting is practicable.
PART  -up -super -gpt -cmp 1  //to compress the GPT partition table only,making partition ID
                                successive starting from 1.
PART  -super -up -swap:2 3#1   //3#1 swap with 3#2
PART  -up -hup -super -force -mbr init 1 //to initialize into MBR; -gpt:into GPT.
```

## Remarks

- Disk operation,belonging to the ones of system level,is very dangerous,so be cautious!

