# BLOCK

To define a segment of code.With its own stacks,it automatically destructs PE variables,
locks,controls and HKEY when it exits.It can be used as an internal function.
{ being followed by * indicates a code segment of THIS type without stacks of its own or the
ability of self-dustruction,merely indicative of range.

## Syntax

```text
A segment of code surrounded by{}
 TEAM SET &cmd={CMD1 %NL% CMD2 %NL%...}|%&cmd% //Multiline with variable embedded,separated
 by line breaks.
 (CMD1 %NL% CMD2 %NL%...)  //Multiline with plain codes embedded,separated by line breaks.
```

## Examples

```text
TEAM { wait 10 | LOCK train tickets | handle train tickets  123 | handle train tickets 2 }
```

## Remarks

- It can be used at file level or in a function and a command group.Nesting is permitted.
- At file level or function level,{} should be the first non null character in the first
  line.At command group level,unrestricted,yet{} should be at the head of the command group.
- Condition Block
  IFEX/FIND  COND,[!]       //With ! refers to ELSE part,without ! or !! refers to IF part,
  only one of them is adopted.
  { ....code lines
  }
  IFEX/FIND  COND,[!] { CMD1   // "{" is not at TOL.Be cautious to use.It cannot be nested.
  ....CMD2... code lines    //True[False]Condition
  }
  IFEX/FIND  COND,    //without !
  { ....code lines    //True condition
  }!
  { ....code lines    //False condition,"}" is closely followed by !,! can be followed by
  several null characters,and then by "{".
  }
  IFEX/FIND  COND,  { CMD1   //without !,"{" is not at TOL.Be cautious.It cannot be nested.
  ....CMD2... code lines  //True condition
  }!
  { ....code lines    //False condition,"}" is closely followed by !,! can be followed by
  several null characters,and then by "{".
  }
  IFEX/FIND  COND, CMD!  //True condition,! is followed by blank.
  { ....code lines       //False condition.
  }
  LOOP/FORX  COND,[!] [FORX Var,]
  { ....code lines
  }
  Notice:Nesting is permitted within a block.{}should be at TOL,where multiple {} can be mixed
  {...} can be embedded into a line as a whole.IFEX/FIND/LOOP/FORX  COND,[!!]  ...{...}...

