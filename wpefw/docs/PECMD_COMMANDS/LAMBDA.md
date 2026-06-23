# LAMBDA

To define a segment of code with stacks and param list of its own.It can be used as an
internal function.It automatically destructs PE variables and locks when exits.

## Syntax

```text
[]param list separated by space{LAMBDA function body}
```

## Examples

```text
[]P1  P2  P3  "345        667"{ MESS [LAMBDADEMO] %* @DEBUG #OK }
```

## Remarks

- It can be used at file level,in a function or a command group.Nesting is permitted.
- At file level or function level,{} should be the first non null character in the first
  line.At command group level,before | or at EOL,yet{} should be at the head of the command
  group.
- LAMBDA function body of command group level will be interpreted as a variable string,so it
  is dynamic,and meanwhile "%" should be replaced with "%%".
- With stacks of its own,it automatically destruct PE variables,locks,ctrls and HKEY on exit
- _SUB is actually LAMBDA and LAMBDA is featured to be able to use the stacks of its caller.

