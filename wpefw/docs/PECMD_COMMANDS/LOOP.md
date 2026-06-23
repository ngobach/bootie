# LOOP

To check the loop conditions and execute the loop commands.

## Syntax

```text
LOOP <LoopCond>,[!]<LoopCMD>
LOOP {! }{$#}[{! }<COND1> relation operator {! }<COND2> ...],{!}<LoopCMD> //combined
conditions
```

## Parameters

- **LoopCond**: The format is [$#]Var1[<comparison operator>Var2],'$' indicates string comparison
  (not case sensitive); or 'leftpart'<comparison operator>'rightpart','#' refers to integer
  comparison(INT64).The Leading character being absent or | indicates comparison of Double
  varialbe.
  ☼comparison operator:"<",">","=","!",">=","<=","!=","<>" respectively indicates:less than,
  greater than,queal to,unequal to,great than or equal to,less than or equal to,unqual to,
  and equal to.
- **LoopCMD**: '!' indicates inverting the outcome of the condition judgement first.The command
  will be executed in a loop until the condition holds.
  ☼combined conditions:[] are their symbols,and () can be nested.S| before[ indicate all
  behind being $| comparison.S| can be absent.|&@ are relation operators.There should be
  spaces between combined conditions,as well as before and behind the relation operators |&@

## Examples

```text
LOOP #%Number%<100,CALL DoLoop  or LOOP $%Characters1%=%Characters2%,!CALL DoLoop
LOOP [ COND1 & ! COND2 & COND3 ], DoSth           //example of combination.
LOOP [ #2 < 12 & $7 < 8 & 4 < 10 ],  MESS LOOP1   //example of combination.
```

## Remarks

- There should be a command to modify the varialbe in the LoopCMD,or the loop will not end
  (endless loop); It is suggested placing the loop body in the sentences defined by _SUB and
  _END.

