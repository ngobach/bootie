# SUBJ

To associate the path with the drive number,equivalent to the command of SUBST in CMD.
-BD[-] Notify Add/Remove drive letter

## Syntax

```text
SUBJ [-BD[-]] [*&-]<VirDrv>,[Path_assigned_to_VirDrv|?RetName]
```

## Parameters

- **&**: local mode,may not be seen by other processes.When mounting,* indicates lasting mounting
  and when unmounting,* indicates unmounting all.Otherwise it refers to the last mounting.
- **If "Path assinged to VirDrv" is absent or - is present,the specified drive will be deleted**
  Chinese characters can be used for the virtual drive.
  The path can be one for a physical device,like \Device\HarddiskVolume7
  \\.\CDROM0~N \\.\TYPE0~N \\?\Volume{6467fdff-fe6d-11d5-8ba2-0000e232d2e9}\.
  A drive letter can be continuously mounted to different positions.The seen is the last.
  Unmounting without * will see the opposite procedure.Unmounting with - means unmounting
  the specified target behind.Chinese characters help to settle the insufficiency of drive
  letters and keep secrect and can be mounted or unmounted as is needed.One device can be
  mounted more than once.

## Examples

```text
SUBJ B:,D:\My or SUBJ *Mao:,\Device\HarddiskVolume7 or
SUBJ *Mao:,\Device\Harddisk0\Partition1
```

## Remarks

- When a drive is to be virtualized,it should not be an exsisting one.When a virtual drive
  is to be deleted,the drive letter should be correct,or a physical one is likely to be done

