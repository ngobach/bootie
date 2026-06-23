# NTPC

To obtain and synchronize time.

## Syntax

```text
NTPC [-q|-qo] [server][,name]
```

## Parameters

- **-q**: to synchronize and query,-qo:to query only,and save the outcome to the variable name.
  No -q or -qo indicates synchronizing only.No server indicates obtaining the default time.

## Examples

```text
NTPC -q   ,CurrentTime
```

