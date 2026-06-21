# SITE

To set or clear the attribution of file and directory,supportive of 4 types of attribution
A,H,R,S. FTIME is compatible with DATE,with the format of *local0.

## Syntax

```text
SITE <FileDirPath>,<FileDirAttr>
SITE *touch[:[cr][*local:|*local0:|*sys:|*sys0:|*utc:]Time],<FilePath>[,RetVarName]
                                                         ☼to update the time of the file.
SITE ?VarName[,ProduVerName]=FVER,FileName               ☼to query the version of the file.
SITE ?[-local -ws -link] [[*]CreaTimVar][,[*]WritTimVar][,[*]AccTimVar]=FTIME,FileName
                                                         ☼to query the time of the file.
SITE ?[AVarName][,HVarName][,RVarName][,SVarName][,FullVarName]=FATTR,FileName
                                                         ☼to query the attribution of file.
```

## Parameters

- **To set the attribution,use "+",and to eliminate,use "-",(A=DOC,H=HIDDEN,R=READ-ONLY,S=SYS)**

## Examples

```text
SITE %SystemDrive%\BOOT.INI,+H+R
```

## Remarks

- SITE is capabile of setting the attribution of both file and directory.* before TimeVar
  indicates returning an integer of UTC time,which is directly applicable to comparison
  operation,otherwise a string is returned with all sections of the same width in the form
  of YearMonthDayHourMinuteSecondMicrosecondWeek.FVER/FVER/FATTR is mapped to the command of
  ENVI/SET.
  The variable of AHRS returns 0 or 1; FullVarName returns all attributions(more than 4)in
  hex; Null if fails.
- -local:to return the local time and world time by default.-ws:to add a sequence number of
  the week in a year to the end of the returned string. -ws1:Sunday as the weekend.

