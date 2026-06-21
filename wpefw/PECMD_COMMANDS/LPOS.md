# LPOS

To return the position of the specified "Char" from the left of the "Str".
* indicates a VarName behind. ~indicates figuring out the position from right.

## Syntax

```text
LPOS[*] [*] [-case] [[*][~]VarName][,[~]LineVarName]=Char,Num,Str //prefix* for many
LPOS**[*][#$] [-qu] [-delims:Separator] [-case] [[~]VarName][,[~]LineVarName]=SubStr,Num,Str
```

## Parameters

- **VarName**: to store the outcome of the command; -case:case sensitive.
- **Str**: the string to check,any character or space permitted.
- **Char**: the character to check,not case sensitive,any character or space permitted.Multiple
  characters will be treated in the substring mode.If the substring is",",it should closely
  follow =.
- **Num**: to specify the number of times the character to check appears,1 by default.
- **LPOS***: It is the SubStr to search for,ie,the part between "=" and ",",any character or
  space permitted.
- **LPOS****: It is the SubStr to search for and return a string number separated by space.
  number>=1;  *** or -qu:to be compatible with MSTR;  $:only space;  #:only TAB,or -delims:
  to specify the separator(\n\r\t\v\f\b supported).

## Examples

```text
LPOS iPos=a,2,123A56|1234A6|abcdef or LPOS iPos=a,3,123A56|1234A6|abcdef
LPOS*** &&P2="BB FF 199",,AA "BB FF 199" DD
```

## Remarks

- If "Num" is smaller than 1,the command will return the position of the character to check
  from the right of the string,eg,the outcome of the example above is 15; If the returned
  outcome is 0,that means no specified character is found.
- This command will treat the string in Unicode.The returned value of example 1 is 1 and
  example 2 is 15(the same as the outcome when Num is 0).
- The character starting from the second one of the substring cannot be ",",but the first
  one and the character mode can be ",".It can be transfered to a variable in that case.

