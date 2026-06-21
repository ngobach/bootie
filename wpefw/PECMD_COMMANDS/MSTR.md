# MSTR

To cut out a string of specified length from a specified position in a specified "Str"
towards the right and return the outcome to a specified variable.* indicates a VarName
behind.

## Syntax

```text
MSTR [*] VarName=Pos,Length,Str
```

## Parameters

- **VarName**: to store the outcome of the command;
- **Str**: the source string,length unlimited;
- **Pos**: number,to specify the starting position of the source string;
- **Length**: number,to specify the length for cutting out;
- **StrID**: ID>=1,to return the string according to the ID number.Strings are defined by space
  or ".The pair of <> should not be left out.~ indicates removing the outermost pair of ".
  * behind MSTR indicates separating with TAB; $ separating with space.-left:to retain null
  characters at the very beginning of the string.(the TAB mode by default)

## Examples

```text
MSTR aStr=2,4,1234567890 or MSTR aStr=<~2>1 "a b c" 3 or MSTR aStr=-2,4,1234567890
MSTR a,b,c,d=<4><3><5><2>1111  222 333  444  555 666
MSTR a,b,c,d=<2*>1111  222 333  444  555 666  or MSTR a=<1>"PROG \"%L%\" "  1234
MSTR Q1,Q5,Q6,Q6x=<1><5-><6><6->%vSTR%      MSTR Q1,Q5,Q6=<1-><5-><6>%vSTR%  //mixed
```

## Remarks

- If "Pos"<0,the cutting out will be processed starting from the tail in reverse order;
  If "Length"<=0,real_length = total_length_of_real_surplus - absolute value of "Length"
- If "Pos"=0,it will be treated as 1,and if "Length" <1 or exceeds the length of the source
  string,all characters behind the starting position will be returned;
- This command will treat strings in Unicode.So the returned value of example 1 is 2345,and
  example 2 a b c.
- In format 2,the variables in VarName list will be assigned respectively according to the
  positions in StrID list.
- Format 3 is a simplized form of format 2 to get a successive list of StrID.
- Format 4 indicates assigning the whole part starting from the StrID to VarName.
- Fortmat234 all support cutting out characters in reverse order.-1 indicates the last one.

## Model / Use Cases

1.The value of a varible(or a certain line of the text)like 5555    66  uuty    00  pp ww
  is broken by one or more spaces(the number of spaces is uncertain,but one at least)into
  several sections,the lengths of which are also uncertain.
ENVI &STRING=5555 66 uuty 00 pp ww
MSTR V3=<3>5555 66 uuty 00 pp ww               ///uuty
MESS V3=[%V3%]@#OK
MSTR V3=<3>%&STRING%                           ///uuty
MSTR V_3=<-3>5555 66 uuty 00 pp ww             ///00
2.HELP LAMBDA  to separate the param automatically  //LAMBDA code pattern of the model
ENVI &&V3=
[]5555 66 uuty   00 pp ww{  ENVI &V3=%3}    ///no null characters before },or it will be
                                            treated as part of &V3   MESS V3=[%&V3%]@#OK
[]5555 66 uuty   00 pp ww{  TEAM ENVI &V3=%3|  }   ///There are null characters before },
                                                      and | is the end of &V3.
MESS V3=[%&V3%]@#OK
3.HELP _SUB function to separate the param automatically  //_SUB code pattern of the model
_SUB  GETV3
  ENVI &V3=%3
_END
ENVI &&V3=GETV3 5555 66 uuty 00 pp ww       ///uuty
MESS V3=[%&V3%]@#OK

## Sub-specification: 2

MSTR[*$] [*] [-xq] [-left] VarName1,VarName2...,VarNamen=<[~]StrID1[-]>...<[~]StrID2[-]>Str

## Sub-specification: 3

MSTR[*$] [*] [-xq] [-left] VarName1,VarName2...,VarNamen=<[~]StrID1*>Str

## Sub-specification: 4

MSTR[*$] [*] [-rq[1]] [-trim[p][left|right]] VarName=Str //to remove [prepositive] white
 space character at head and end,$:only space.
*:only TAB,-delims:UNICODE delimiters to specify a special delimiter; -xq indicates \" can
be contained in the string; -rq:to romove ""; -rq1:to contain the single ".
-term:UNICODE delimiters left. StrID # Returns StrNum    -1 add double quotes on spcae
-trim*+[+]  -rq*+[+]  trime before[after]  oprating for -trim -trimleft -trimright -rq -rq1.
-trimp:Pretreatment.

