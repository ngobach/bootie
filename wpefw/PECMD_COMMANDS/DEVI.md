# DEVI

To search for(install) the driver from CAB,INF or a specified directory.
☼*dummy indicates not installing really but only for debugging,like viewing DevClass.
  *devid:DeviceID,to install specified driver.
  *nnverify:not to disable driver signature veryfying; *rverify:to restore the setting of
  disabling driver signature veryfying. *7pe[-]:to use Drvload by force.
  *inner:not to use Drvload by force. *drvload/*devcon:to use drvload preferentially(default
  )/devcon.*retid:VarName,to return the device ID of this installation.
  *auto:to automatically shift between whether to handle INF or not(The default is to handle
  for CAB.Notice that INF may be overwritten). *sys:to copy to the system directory.
  *cab:force to be cab file.  *comp+: find ID compatible also. *ret:RetName Report
  *IdCah:PeVar: Cache of ID for times    *infcache:InfCacheFile   *optsys[:v] Systool first
  *num:Times

## Syntax

```text
DEVI [*rescan] [*norescan] [*devclass:DevClass] [$]<CABPath>[,MatchLevel[,ExtractDir]]
DEVI [*rescan] [*norescan] [*nocheck]   <INFPath>[,DevClass]
DEVI [*rescan] [*norescan] <DirWithSubdirOfINF>[,DevClass]
DEVI [*rescan] [*norescan] <*sub|*self> <DirWithINFFile>[,DevClass[|OptClass]]
DEVI $[$]<INFPath>,[InstSection],[ActionCode]        <===StandardInstall,DefaultInstall,132
DEVI [*rescan] [*norescan] *extract <CABPath>,[MatchLevel],ExtractDir   <===pure extraction
DEVI [*rescan] [*norescan] *nocheck <CABPath>,[MatchLevel],ExtractDir,INF relative path
                                                <===according to INF in the cabinet directly
DEVI *rescan[:Fun]                              <===to rescan hardware changes
DEVI listdev:VarName [*devclass:DevClass] [*ALL] [*listdev=i|c|+] [*devid:[h|c|+|@:]ID]
            [*comp[+]] [*hwid] [*inst] [*many] [*rescan]  //to list can be'preID1''preID2'
DEVI *[*]disverify                           <===to disable driver verifying,** as by force
DEVI <*enable|*disable|*remove|*restart>:[h|c|+:]DevInstance/(hc)ID <===to enable/dis... dev
DEVI <*update|*install>:HWID:INF <===update/Install dev.Can be 'HWID1:INF1''HWID2:INF2'...
DEVI *status:RetNm:[h|c|+:]DevInstance/(hc)ID  [*nl]  <===query   can be'ID1''ID2'...
 status:0x400=Problem 0x8000=PRIVATE 0x8=STARTED        PROBLEM:0x16=DISABLED
DEVI buildcache:  [-a: arch] DrvDir  //Build Inf auto-cache. ManualRef  *infcache:CachFile
```

## Parameters

- **To specify the CAB path**: "$" before CAB indicates installing driver after extracting it,or
  else not installing; can be embedded.
- **"$" before INF indicates executing standard mode of copying,or else only loading driver**
  (likely not to load if the device does not exist);
- ***nocheck**: to install by force without checking hardware ID;
- ***rescan[**: Fun]:to rescan hardware changes(the default for installing NT6 by no force).It
  can be used alone;  Fun:0,1,2,3,4,5,7. *rescan:3 or 2
- ***norescan**: to disable rescanning hardware changes;
- ***showdev**: display dev.
- **MatchLevel**: the number of sections for DeviceID,separated by &,2 by default;
- **To query**: DeviceID by default,*listdev=c:ID compatible,*listdev=i:unique ID;
  *ALL:including devices not existing.
- ***Surport '"str1""str2""str3"'**

## Examples

```text
DEVI *retid:&&IDS $%SystemRoot%\DRV.CAB  or DEVI MyInf*.INF or DEVI %SystemRoot%\Inf\*.INF
DEVI %SystemRoot%
DEVI *extract myCab.CAB,,D:\ABC
DEVI *dummy *sub  D:\ABC                  //to print DevClass of all INF in D:\ABC to LOGS
DEVI *sub  D:\ABC,Net                     //to seek and install network driver in D:\ABC
DEVI *devclass:Net  $D:\DRV.CAB,,%TEMP%   //to seek and install network driver in D:\DRV.CAB
DEVI *devid:PCI\VEN_8086&DEV_1229 $D:\DRV.CAB,,%TEMP%             //network card Intel8255x
DEVI *disable:PCI\VEN_14E4&DEV_16B5&SUBSYS_05001025&REV_10\4&14C08592&0&00E3 //to disable
     network card BCM.
     DEVI $$D:\MyDrvs\Drv1.inf            //♦standard installation mode,DefaultInstall,132
```

## Remarks

- This command applies self-defined(nonsystem)driver-seeking algorithm,capable of extracting
  the cabinet quickly for the driver possibly needed,and thus more drivers than needed are
  likely to be sought for one device.
- In the CAB,be sure to put each driver in a separate directory and ensure the INF file is
  always in the very front of current directory and it has been treated in advance.It is
  suggested using the matching program XCAB to produce it.
- If the drivers are in the form of CAB,'$' and [ExtractDir] cannot be set at the same time
  in NT5.x OS(unless in the mode of NT6.x or *sys),but they can be absent together.
  If '$' is absent,the command will only extract files; If [ExtractDir] is absent,DEVI will
  extract the driver in a driver-matching way.INF files will be extracted to "%WinDir%\INF",
  SYS to "%WinDir%\SYSTEM32\DRIVERS",and other files to " %WinDir%\SYSTEM32".
- If the drivers are in the form of CAB,in NT6.x OS,with '$',[ExtractDir] is a temporary
  directory in %TEMP% if it is null or *(So it is with *sys).The directory will be deleted
  automatically after drivers are installed.If [ExtractDir] is present,'$' can be absent.
  ENVI will extract files in CAB to a specified directory according to the directory
  structure of the CAB.With '$' indicates installing the driver in the destination directory
  after extraction.If not to install,the param *extract is needed.
- If other files are to be extracted to a specified directory,replace the directory
  separator in the file name with "#",eg,files in "SYSTEM32#WBEM#MOF#XXX.MOF" will be
  extracted to "%SystemRoot%\SYSTEM32\WBEM\MOF\XXX.MOF".
- In NT6.x WinPE,DEVI will directly install "%WinDir%\INF" without veryfying the matching of
  hardware ID.
- Another function of this command is to seek drivers from local disks,like "DEVI \Windows,
  Display". The command is not perfect yet,and it will provide a dialogbox for driver files.
  Hence,for this function,technological supports are not available.
  *self indicates seeking *.inf in the directory with INF file; *sub indicates seeking in
  the directory with INF file as well as in its subdirectory; *self*sub both absent means
  seeking in the directory with INF\*.inf
- DevClass: Battery CDROM Computer DiskDrive Display FloppyDisk HIDClass Image Infrared
  Keyboard MEDIA Media Monitor Mouse Net PCMCIA Ports Processor SCSIAdapter System USB
  Volume fdc hdc
- %&__DEVI.REBOOT%: Need reboot    place * of  ID with^*(or^).

