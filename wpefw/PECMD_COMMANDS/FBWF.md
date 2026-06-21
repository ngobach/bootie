# FBWF

To set FBWF cache.

## Syntax

```text
FBWF [P Percent of free memory] [L Min] [H Max] [F SurplusVal]
```

## Parameters

- **The units for Max,Min and surplus value are all MB.**

## Examples

```text
FBWF P20 L32 H64
```

## Remarks

  The three params can be used separately.If FBWF L64 is used,64M of FBWF will be set by force
  ignorant of the size of the free memory.

