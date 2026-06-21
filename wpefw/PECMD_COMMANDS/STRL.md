# STRL

To return the length of a specified "Str".* indicates a VarName following behind,and if not,
-1 will be returned.
  -m/8:to return the number of bytes of a multibyte/UTF8 code,2/(>=2) for a Chinese
  character generally. Otherwise a UNICODE number will be returned,1 for a Chinese character
  .[-m is generally not used].

## Syntax

```text
STRL [*] [-m|-8] <VarName><=><Str>
STRL -bin <VarName>=PEVarName  //to return the number of bytes of the memory for an object
```

## Parameters

- **VarName**: to store the outcome of the command;
- **Str**: the string to check,length unlimited.

## Examples

```text
STRL iLen=1234567890 or STRL dLen=ABCDEFGHI
```

## Remarks

- This command returns the length of a Unicode string.The returned value of example 1 is 10
  and example 2 is 5.

