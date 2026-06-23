# RSTR

To cut out the specified number of characters from the right side of a specified "Str" and
return the outcome to a specified variable.            * indicates VarName following behind.

## Syntax

```text
RSTR [*] <VarName><=><Nunm>,<Str>
```

## Parameters

- **VarName**: to store the outcome of the command;
- **Str**: the source string,whose length is unlimited;
- **Num**: the number of the characters to cut out.

## Examples

```text
RSTR aStr=2,1234567890 or RSTR aStr=5,1234567890
```

## Remarks

- If "Num" is smaller than 1 or exceeds the length of the source string,the returned outcome
  will be the whole source string(the same as the string copy);
- This command will treat the string in Unicode.The returned value of example 1 is "90",and
  example 2 is "67890".

