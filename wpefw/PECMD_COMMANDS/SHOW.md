# SHOW

To display removable disks,the hidden partitions on fixed disks or unmounted partitions
existing in the system and assign drive letters to them.

## Syntax

```text
SHOW [=1] [-check] [-SKIP=Type] [-skiptp:Tp1;Tp2] [-skippt:hd1:lpt1;hd1:lpt2]
     [-from:DrvChr[Tbl]]    [*&-] [DiskPart],[UDrvChr[Tbl]],[TimeWait],[StartDrvChr[Tbl]]
```

## Parameters

- **&**: local mode,unlikely to be seen by other processes.When mounting,* means lasting mounting
  and when unmounting,* indicates unmounting all.Otherwise it refers to the last mounting.
  - indicates unmounting.=1 indicates not mounting if mounted.-check:not to mount if NA.
- **DiskPart**: consisting of [HDNo or ID:PartNo or ID#PartNo].Either "HDNo" or "ID" is adopted.
  ♦HDNo:number,< 0 indicates hard disk,U disk and removable hard disk all;
  0 refers to "hd0",1:"hd1",2:"hd2",3:"hd3",and so on;
  ♦ID:indicated as "F" or "U"."F" refers to the fixed hard disk,"U":u disk or USB removable
  disk;
  ♦PartNo(logical):number,< "0" indicates all partitions without drive letters,"0":all
  visible partitions without drive letters,"1":the first partition,"2":second,"3":the
  third,and so on.Being absent or - indicates deleting.The one behind # refers to physical
  partition number.1~4 as the main partitions and 5~N as the logical partitions.
- **DrvLetter**: indicated as "A~Z",If absent or unproperly set,it will be automatically
  assigned.Chinese characters are permitted,which help settle the insuffiency of drive
  letters and keep secret(cannot be listed).They can be mounted or unmounted as is needed.
- **TimeWait**: number,indicating waiting for a certain period of time (ms)for devices getting
  ready to be assigned with drive letters.

## Examples

```text
SHOW 0:1,H or SHOW U:1,U or SHOW F:0 or SHOW U:0,,3000 or SHOW U:-1 or SHOW 0:-1 or SHOW
SHOW 0#1,H or SHOW 0:1 or SHOW 0#1 or SHOW - 0:1 or SHOW - 0#1
```

## Remarks

- Computers of some types can recognize the removalbe hard disk after system starts up,yet
  has not assigned a drive letter to it.This can be fulfilled by using the command of SHOW
  with the identifier of "U".Example 2 shows assigning the drive letter "U:" to the first
  partition of the removable hard disk.
- Example 1 shows "HD 0 Part 1" is assigned with "H:" and example 3 shows automatically
  assigning drive letters to all the visible partitions on the fixed hard disk;
- Usually,"TimeWait" can be absent.It is internally used when PECMD residenting in memory
  has detected the insertion of the removable hard disk;
- The command of SHOW with all params absent will not voluntarily mount the hidden partition
  unless the "PartNo" is set as "-1";
- Use SHOW or SUBJ to delete the drive letter so as to load the outparts in the hidden
  partition and keep its attribution of being hidden at the same.

