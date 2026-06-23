# IMPORT

To directly import another module of source file,making up a large file in logic.

## Syntax

```text
IMPORT FileName
```

## Parameters

- **FileName**

## Examples

```text
IMPORT  TrainModule.wcs
```

## Remarks

- It can be nested with the function of anti-loop inside.
- It is placed at the head of a file and import codes mechanically before they are executed.
  It should be a unique command in a separate line.It is not influenced by FIND/IFEX/_SUB.

