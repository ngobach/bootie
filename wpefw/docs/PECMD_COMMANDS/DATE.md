# DATE

To returen current system date and time
  *h:high-accuracy,*r:synchronous high-accuracy timer,at the very head of the param list.
  *space indicates outputing space,with 0 indicates completing with 0; *bsys:to output
  international time; Other params indicate not obtaining current time but converting the
  time.*local *sys followed by 0 indicates the input in the format of *space0.
  *us indicates millisecond with 4 decimal places.

## Syntax

```text
DATE⌠*h⌡⌠*r⌡[*space[0]] [*bsys] [*utc:UTC time] [*gmt:GMT time] [*local:LOCAL time]
     [*us] [*sys:international time]                       [VarName]  [SubItem]
```

## Parameters

- **VarName**: to store current system date and time.CurDate by default.
- **SubItem**: y Mon d w h min s ms ws[1] ds Freq Counter respectively indicates year month day
  week hour minute second millisecond NumberOfWeeks xxth_day_of_year frequency counter.
  Also directly adopt the subitem of Chinese version yy MM DD ww hh MM SS MS W[1]day.
  All subitems will be returned if unspecified.The subitem gmt indicates the number of
  seconds GMT(timing from 1970.1.1),uptime refers to the number of milliseconds of the PC
  running time,utc refers to UTC time with the unit of 100 nanoseconds,timing from 1601.1.1.
  uptimens refers to the number of nanoseconds of the PC running time. Counter refers to
  high resolution timer Counter. Multiple subitems are ermitted.

## Examples

```text
DATE *us SysDate
```

## Remarks

- The returned outcome will be stored in a specified variable in the form of "Y-M-D|W|H:M:S"
  ,the date and time of which are ready to be cut out with MSTR for separate use.
- If VarName is absent,the outcome will be stored in the environment variable %CurDate%,the
  possible outcome of the example is "2008-8-8|5|20:8:8".
- International time and LOCAL time can be bracketed with a pair of quotes/double quotes,and
  each item is separated by any nonnumeric character.

