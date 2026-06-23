# TEAM

To sequentially pre-execute all the commands in the specified command group. ^#~+- are the
alternative delimiters for |.  <: right to left.

## Syntax

```text
TEAM[^#~+-@$%*?/\;,."'<] CMD1|CMD2|CMD3|CMD4|CMD5|......|CMDn
```

## Parameters

- **one or more commands,separated by "|".**

## Examples

```text
TEAM TEXT Load Desktop|LOGO|HIDE|SHEL %SystemRoot%\EXPLORER.EXE|WAIT 3000
```

## Remarks

- This command cannot be nested with IFEX or FIND.
- TEAM nest:The outermost is broken by |,the inner by ||,and the further inner by |||...

