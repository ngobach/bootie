# PAGE

To set the page file(vitual memory).

## Syntax

```text
PAGE [*force] <PageFilePath> <InitSize> [Max]
```

## Parameters

- **Use the format of DOS 8.3 to specify a path to the page file,like "C**: \PageFile.sys".
  The unit of InitSize and Max is MB; If Max is absent,Max will be automatically set the
  same as InitSize.

## Examples

```text
PAGE C:\PAGEFILE.SYS 128 256
```

## Remarks

- If a page file is set,this partition cannot be formatted.
- If the memory available > InitSize,this command will not set a page file,ie,the defined
  InitSize can be used as the condition for setting a page file.
- This command will intelligently search for the qualified(hard disk and capacity) partition
  to set the page file starting from the one containing the defined name of the page file
  (including the drive letter) to avoid those slow u disk or removable hard disk.If the
  system starts up from u disk or removable hard disk,the page file defined in the config
  file is likely to be right in them.After a round of search,only if no qualified hard disk
  partition is found will the u disk or removable hard disk be employed to set the page file
  *force:to disable the function in REM2 and REM3.

