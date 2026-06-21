# CALC

To store the outcome of the expression calculation in a "Var".E:scientific notation;
F:decimal; G:to adopt a brief way automatically.

## Syntax

```text
CALC[-txt-cb-[Lfr:Lto[:Rfr:step]]] [-gui -base=2|8|10|16|N] [-err=ErrVal]
                                                  [[[#]Var]=]Expr1[#[#][DeciDigits][E|F|G]]
CALC Format[[[#]Var]=][[#|$]SubVarName1=]Expr1 and suffix;or newline[[#|$]SubVarName2=]Expr2
     and suffix; ...
```

## Parameters

- **-base= indicates the base of the output.Outcome**: binary is indicated as 0bNNNN; octonary as
  0oNNNN; hex as 0xNNNN; These datas can be directly operated by CALC.N,as other nonstandard
  number system,carries no prefix and cannot be operated.The number system is also
  applicable to floating number(rounding). -gui:graphic users interface
- **The leading charater "#" indicates all variables will be treated as the type of int64; If**
  the leading character is absent or $ is present,all variables will be treated as type of
  double.
- **Specific numbers or existing variables can be in the expression,supportive of the input in**
  scientific notation.
- **Operator supported**: or"||"; and"&&"; xor"@@"; bit or"|"; bit and"&"; bit xor"@"; =,<>; >,
  >=,<,<=; "+","-"; mul"*",div"/",com"%"; pow"^" and ()[]{},arranged in successive priority.
  The number of items is unlimited and the data includes floating number;
  The suffix TGMKS is supported(K=1024 S=512),so is the mathematical constant:
  e(natural constant),pi(circum ratioπ),2 constants in all. A space follows opertor -.
- **Suffix "#"**: When the outcome of calculation is treated as type of double,as many decimal
  digits as possible are kept by default,16 digits at most;(round up or down)
  The meaningless 0 behind will be all eliminated by default; ## indicates retaining them.
  Mathematical functions supported:
  abs(a)   absolute    arccos(a)          arcctg(a)
  arcsin(a)            arctan(a)          ceil(a)  ceiling
  cos(a)               ctg(a)             deg(a)
  div(a,b) division    exp(a)             frac(a)  fraction
  floor(a)             hypot(a,b)         int(a)   integer
  lg(a)                ln(a)              log(a,b)
  max(a,b)             min(a,b)           mod(a,b) modulo
  pow(a,b) power       pow10(a)  10 power rad(a)
  rand(a)  random      round(a)           shl(a,b) shift left
  shr(a,b) shift right sin(a)             sqrt(a) square root
  tan(a)               xor(a,b)           not(a)[or ~a]
  lnot(a) [or !a] logical not

## Examples

```text
♦ CALC #Sum = 128 + 32*22 or CALC Sum = %Datum1% + %Datum2% * 32
♦ CALC #Result = 128 * ( 64 +66 ) or CALC Result = %Datum1% * ( %Datum2% + 12 )
♦ CALC 1+3 or CALC V=  &v1=1+3;%&v1%*100 or CALC =  &v1=1+3;%&v1%*100
```

## Remarks

- "Var"can be directly set by ENVI(assigned)or the assigned outcome of CALC calculation.
- IFEX can also be employed to judge and compare numerical values.
- No difference in handling float and integer numbers below 16-digit decimal.
  ±9007199254740991
- To use % conveniently,%n%* are forbidden.If they are needed,convert them into variable
  referencing with ENVI, or add ^before cmd.If CALC | is used in TEAM,More "|" should be
  added to according to the number of nesting for "|".
- To obtain a random number:directly reference %RANDOM%,the outcome of which is 63-digit
  positive integer different each time.RAND VarName or ROUND() is also practicable.
  0----9:CALC #R0_9=%RANDOM% % 10 or RSTR R0_9=1,%RANDOM% or ROUND(10)
- If VarName is absent,the outcome will be displayed.Expressions are separated by ";" or
  line breaks.
- The limit of integer in CALC and PECMD ranges from -0x8000000000000000~0x7FFFFFFFFFFFFFFF
  No such limit for a float.
- -[f1:t1[:f2]] operation for vector. %%d is cursor.The parameters are left start_end and
  right start

