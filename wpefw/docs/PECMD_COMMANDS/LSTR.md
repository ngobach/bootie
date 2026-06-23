# LSTR

To cut out the specified number of characters from the left of a specified string and return
the outcome to a specified variable. * indicates a VarName behind.

## Syntax

```text
LSTR [*] <VarName<=><Num>,<Str>
```

## Parameters

- **VarName**: to restore the outcome of the command;
- **Str**: the source string,lenghth unlimited;
- **Num**: the number of the characters to cut out.

## Examples

```text
LSTR aStr=2,1234567890 or LSTR aStr=5,1234567890
```

## Remarks

- If "Num" is smaller than 1 or exceeds the length of the source string,the returned outcome
  will be the whole source string(the same as the string copy);
- This command deals with the string in Unicode.The returned value of example 1 is "12",and
  example 2 is "12345".

