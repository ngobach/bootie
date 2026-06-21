# SED

To substitute or search a regular expression. -t:translate character set.  -ts[1]:translate
string set(Chr(13) separated[whole]).  -ex [-ex [-ex]]:SourceString[RegStr[SubstituteStr]]
is a varname. -[Lf:Lt[:Rf]]for vector  -h[~]?LeadStr_UnlessHead[NotInclude]
-e[~]?EndStr_UnlessTail[NotInclude]

## Syntax

```text
SED [-[Lf:Lt:Rf:Stp]] [-ni] [-t[s]] [-ex] VarName[;TimesName]=[TimesSubs][:TimesSkip][Mark],
    SearchRegularExpression,SubstituteSubstring,SourceString        //substitute.
SED [-ni] [-many] VarName[;TimesName][;LineName]=?[Times][:SkipS][MarkS],FindRegularStr,
    [SubNm1;SubNm2],SourceStr //query position 0:not found. -many/*: return many.-L:SplitLn
```

## Parameters

  Mark:three characters,respectively corresponding to three strings behind. * indicates not
  interpreting(left as the literal is), _ refers to a placeholder. -many/*return Many Pos
  *ni:not case sensitive. \u:to shift to upper case, \l:to shift to lower case.
  -[n1:n2]:vector operation. ~before VarName/LineVarName:to figure out position from right.

## Examples

```text
SED &&V=3:1,[A-Z],=,NiHaoABC or SED &&V=?:1,[A-Z],,NiHaoABC or  SED -t &&V=0,[AB],ab,ABA13
```

## Remarks

  TimesSubs is 1 by default(0 for SubNmS),0 means all; TimesSkip is 0 by default.

