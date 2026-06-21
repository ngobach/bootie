# RPOS

To return the position in which the specified "Char" appears counting from the right side of
a "Str".* indicates VarName following behind.~indicates figuring out position from right.

## Syntax

```text
RPOS[*] [*] [-case] [[*][~][VarName][,[~]LineVarName]=Char,Num,Str //prefix* for many
```

## Parameters

- **VarName**: to store the outcome of the command;  -case:case sensitive.
- **Str**: the string to check; any character or space permitted.
- **Char**: the character to check,not case sensitive; any character or space permitted.Multiple
  characters will be treated in the substring mode.If the substring is",",it should closely
  follow = .
- **Num**: to specify the number of times the character to check appears.
- **RPOS***: It is the SubStr to search for,ie the part between "=" and ",",any character or
  space permitted.

## Examples

```text
RPOS iPos=a,2,123A56|1234A6|abcdef or RPOS iPos=a,3,123A56|1234A6|abcdef
```

## Remarks

- If "Num" is smaller than 1,the command will return the position of the character to check
  from the left of the string,eg,the outcome of the example above is 4; If the returned
  outcome is 0,that means no specified character is found.
- This command will treat the string in Unicode.The returned value of example 1 is 12 and
  example 2 is 4.The substring cannot contain ",",which can be treated as a character.
- The character starting from the second one of the substring cannot be ",",but the first
  one and the character mode can be ",".It can be transfered to a variable in that case.

