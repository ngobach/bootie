# RECY

To set the recycle bin of Windows system,like XP/2003/VISTA/Windows7 and their WinPE system.

## Syntax

```text
RECY <PartID>,<MaxCapacity>
```

## Parameters

- **PartID**: indicated as a drive letter or "*:\".The drive letter indicates a specified
  partition and "*:\" all partitions."\" can be absent.
- **MaxCapacity**: number,indicating the maximum space of the recycle bin.In NT5.x system,it is
  indicated as a percentage while in NT6.x system as the capacity with a unit of MB;
  If it is 0,the recycle bin in the specified partition will be disabled and the deleted
  files will not be transfered to the bin.

## Examples

```text
NT5.x OS: RECY D:,10 or RECY E:,0 or RECY *:,0         NT6.x OS: RECY C:,2048 or RECY D:\,0
or RECY *:\,0
```

## Remarks

- In NT5.x OS,if the MaxCapacity exceeds 30,PECMD will adopt the default 10; In WinPE,it is
  suggested putting this command behind INIT.

