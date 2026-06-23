# CMPS

To compress a file.

## Syntax

```text
CMPS [-f] [-m] [-bin|-src[:Flag]] [-utf8] [-o] [-u] SrcFile [,]  CmpsFile
```

## Parameters

- **-f**: to keep the format mark of the source file,optional.-m:unencrypted,-u:to extract,-o:
  old encryption. -bin:encrypt for binary
- **Flag**: 1:del comment line 2:translate \r\n 4:merge blank line 8:del CommentInLine(danger)

## Examples

```text
CMPS A.wcs  A.wcz   or   CMPS -m B.wcs  B.wcz or CMPS -u B.wcz  B2.wcs
```

## Remarks

  A compressed file can be directly loaded for execution,or treated as an embedded script,its
  icon,bitmap and size unrestricted.Encrypted files can be executed but not extracted.

