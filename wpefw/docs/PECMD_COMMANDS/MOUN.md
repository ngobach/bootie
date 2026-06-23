# MOUN

To mount a certain image in WIM to a specified directory or unmount the mounted one.

## Syntax

```text
MOUN[-svr] [-u] [!][WimPath],<MountDestDir>,[ImgID],[WimTempDir]  //-u:to support WimPath
MOUN -query VarName[=rw][,MountDestDir][,WimPath]
MOUN-udm  ⌠sync⌡ [-ud|-uh|-muh[g]] [-u+] [-udfs] [-udm-] [-w] [-m] [-mall] [-mhide[1]]
    [-findboot[Only][: RetVarName2]] [-CurDrv[R][+]] [-onlys] [-ud[m]id: UDxID] [-mnorhide]
    [-udmask: Mask] [-check[-]] [-ret: RetVarName] [-udimg:File] [-mh: <efi|auto|1-4>]
    [-mw[-]] [-opt[s]:imdisk params] [-CheckFile: FilePath]  DevName [DrvChars]
MOUN-udm  upud  -ret: &&V  -udmask: Attr DevName  UDName Off(B) Len(B,>4G allowed)
MOUN-udm  listudm -ret:RetVarName [-norm[-]] [-udfs [-udmask:flagMask]] [-u+[pre]] [-vhd]
    DeviceName [MultiUDWildcard]
MOUN-udm  listud  -ret:RetVarName [-udmask: flagMask] DeviceName [MultiWildcard] //to return
    the detailed UD file list.
MOUN-udm findboot -ret:RetVarName //to find boot device,return:DiskNum UDMNum Type RawInf
MOUN-udm findudm [-img] [-norm] -ret:RetVarName DrvChr //to find corresponding UDM,
                                                        return:DiskNum UDMNum DrvChr Type
MOUN-udm setboot -ret:RetVarName BootMenu [<UDMDrvChr|DiskNum>[#udmNum]] [Type] [AutoMntDrv]
                                                      //to set windows boot menu, Auto Mount
MOUN-udm ud2fs [-efi] DevName [bClr=1] [bMkNew=1] [FS=FAT] [sz1:sz2...[:*]]   //UDExt to FS
MOUN-udm mapsub [-check] [-CheckFile[+]:Fn] [-tag[+]:Fn] [-ret:RetName] [-r] FileName DrvChr
                                                       //w-mount file on RO UDm
MOUN-vhd [-c[x] -d -u -r] [-s:SectSize] VHDPath,[MountPath|Size|Parent],[ID],[RetName][,AppP
    eVar] //-c[x]:Create[VHDX] -r:RO -d:Dynamic  AppPeVar:Unmount once out of variable range
        // or the process exits. -iso:ISO
MOUN-vhd -query [-r] VHDPath,InOutPEVar[,RetBytesPEVar][,Cmd]//GET_VIRTUAL_DISK_INFO 8B:CMD
MOUN-udm OnlyApp [-noauto] //to check and run single application like one-key ghost.
MOUN-udm SetOnlyApp <-udm:DiskNum [-desc:Note] -bioscmd:biosBootCMD" "-eficmd:efiBootCMD"
    -efimenu:EFIMenu -biosmenu:BIOSMenu]|BootMenu>  Type Func APP AppDisk AppPt  bShutdown
    OpDisk OpPt  pt_sn SrcDsk SrcPt SrcPath [WimName FSType  FSLable PBR BootDsk  BootPt
    grldr WimZx0 HvDesc]
MOUN-udm  setbootcfg BootMenuFile "Val"  [LeadTag]
MOUN-udm  getbootcfg RetName LeadTag
MOUN-udm  Server [-FreshDriver[+|Only] -quit|-tips- -qfree[a|-] -safe -replace] //AuMounSvr
```

## Parameters

- **WimFileName**: Microsoft Windows Imaging Format (WIM) File of WIM format.
- **MountDestDir**: directory to mount WIM to.
- **WimImgID**: ImgID in WIM,indicated as a number.It can be absent if the image file is mounted
  read-only or ImgID is 1.
- **WimTmpWorkDir**: When WimFltr temporary working directory is set,the image file is mounted
  read-and-write.It is suggested setting the directory to the hard disk.* refers to the
  system temporary directory.
- **-query**: query mode.VarName:to return the outcome. -u:to unmount. MOUN-svr:Valid on Logout
- **=rw**: to return the RW mark only(1/0).
- **MountDestDir**: the directory to query.If absent,all will be queried(in multiline string).
- **-dll WIMDLL-PATH**: 
- **-onlys**: to mount by soft-only ignorant of the partition table; -mhide:to mount a partition
  unrecogniezed by windows; -mall:to mount all UDm partitions;  -m:many times;
  -ud or -u:soft unmount; -uh:to remove the drive letter from the partition table;
  -muh:-uh first then mount,unmount soft CDROM intelligently(g:by force); -u+:to support U+.
  -cleartmp[only]:to clear the temporary partition item for UEFI; -udfs:to support UD;
  -udm listudm  -ret RetVarName DeviceName:to list UDm partitions;
  -udm sync  "DrvLetterList"  //to flush data to the memory bank; -udm-:not to operate UDm,
  the mask can be *. -mh:partition talble method;
  -udimg:many times; flagMask:0x20000:UD extension,0x40000:only UD extension,0x80000:to
  check UD extension.
  -ainf:variable name for the cache of the partition table;
  -nbrd[-]:not broadcast [or broadcast] the drive letter;
  -opt[s]:can be used many times  -opts:helps MOUN to rid the params of the quotes.
  -CheckFile:FileNameForValid:can be used many times: list or mount only if there is one.
  -CheckFile+:FileNameForValid:can be used many times: list or mount only if there are all.
  -CheckVol[R]: Volume: can be used many times: list or mount only if there is one. R:By SED
  -CheckUuid[R]: UUID: can be used many times: list or mount only if there is one. R:By SED
  -CheckPtType: Type: can be used many times: list or mount only if there is one.
  -tag:FileNameForValid:can be used many times: mount if there is one.
  -tag+:FileNameForValid:can be used many times: mount if there are all.
  -check[-]:not to mount invalid file sytem[or not]
  -udmid:pt#PhyPartitionNo:Mount partition with imdisk(Read only by default).
  -udmdev:"TypeID:DevID:UDMNum":to specify the device and UDm to mount,or else not to mount.
  If TypeID is DevID,the following is DevID of the disk; If TypeID is devsg,the following
  is disk signature.
  -mnorhide:to mount normal hidden partition as well.
  Type(setboot,findboot,findudm):udm ud u+ pt(normal partition)   UDMNum:PT number/ud file
  -vhd:fix for VHD

## Examples

```text
MOUN %CurDrv%\Outpart\PROGRAMS.WIM,%ProgramFiles%,1  MOUN -udfs -udm- \\.\PhysicalDrive1 R-
MOUN *query RW=rw,D:\Y or MOUN *query MSG1,D:\Y or MOUN *query ALL
MOUN-udm  -mhide \\.\PhysicalDrive* R- or MOUN-udm  -u \\.\PhysicalDrive1
or MOUN-udm  -u "" R
MOUN-udfs  -udm- -mall -udimg: *.img -udimg: *.iso  -udmask: 0xA0001 \\.\PhysicalDrive1 R-
MOUN-udfs  -udm- -udmask: 0x60000 \\.\PhysicalDrive1 R-    //only UD extension.
MOUN-udm   -mall -mhide1  \\.\PhysicalDrive%&hd%  R-       //only UDm
MOUN-udm   -udm- -u+ -mall -mhide1  \\.\PhysicalDrive%&hd%  R-                //only U+
MOUN-udm   -udm- -mall -mhide1  -udfs  -udimg: EFI*.IMG \\.\PhysicalDrive1 R- //only UD
MOUN-vhd  -c D:\A3.VHD,20M      MOUN -vhd  D:\A3.VHD,B:
MOUN-udm SetOnlyApp -reboot -udm:1 "-bioscmd:chainloader /WAIPE/PEXPWIM.BIN"
    "-eficmd:chainloader /EFI/boot/boot${arch0}.efi" "-desc:NiHao" -biosmenu:/WAIPE/PEW
    -efimenu:/EFI/microsoft/boot/bcd  GHO 1  ?:\ZhiMaKaiMen_ZDHuiFu.NTR
```

## Remarks

- The name of file and directory supports interpreting environment variables; FBWF should be
  behind MOUN when it is used.
- Path beginning with "\" indicates searching all drive letters and partitions (including
  the hidden ones) for the file with a specified directory like MOUN \MyPE\OP.WIM,%PF%,1
- "!" indicates assigning a drive letter to a hidden partition after the WIM in it is
  successfully mounted like: MOUN !\MyPE\OP.WIM,%PF%,1
- Leaving out WimPath indicates unmounting the image.ImgID being non zero indicates saving
  the modified contents when the image is unmounted,while 0 not saving.
- The full outcome of query is: ID FLAG "WIM File"  "MountDir",In FLAG,the 0x100 bit is RW.
- MountDestDir is created automatically.If a read-only file is mounted,the mounting will be
  automatically converted to the read-only mode.
- Inner IMDISK needs cabinet.DLL.
- Driver auto mount to can be stored in HKLM\SOFTWARE\PELOGON\USRCFG\AutoUDmMount.DriverTo.
  Can set it at the begin of PECMD.INI.

