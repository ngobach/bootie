# HASH

To calculate CRC32,MD5,SHA1 or SHA256 check codes of a file or string.

## Syntax

```text
HASH [FileName|$StrContent|*PEVarName],[VarName],[ChkType]
```

## Parameters

- **FileName**: the full name of the file whose Hash check code to be calculated,containing its
  path,supporting interpreting environment variables;
- **StrContent**: string,Chinese characters can be contained,case sensitive.Either "FileName" or
  "StrContent" is present;
- **The leading character $**: a symbol for string; If FileName/StrContent contains ",",the two
  "," behind cannot be absent.The string is in ASCII code.
- **PEVarName**: to directly calculate its hash without creating a temporary file.If PEVarName
  contains &,* can be absent.
- **VarName**: to store the outcome of calculation in string.
- **ChkType**: string,"CRC32","MD5","SHA1","SHA256" or "MD5" by default.

## Examples

```text
HASH %SystemRoot%\System32\UserInit.EXE,md5UserInit or HASH $mdyblog.blog.163.com,hPSW,SHA1
```

## Remarks

- When using HASH to calculate the HASH check code of a string,its size is unlimited.
- When an error arises in reading a file,its MD5,SHA1 and CRC32 are all null; The CRC32 of a
  null string is "00000000",MD5 is "d41d8cd98f00b204e9800998ecf8427e",and SHA1 is
  "da39a3ee5e6b4b0d3255bfef95601890afd80709".
- VarName given,the outcome of the calculation with HASH will be stored in a specified
  variable.In example 2,VarName can be referenced by the environment variable %PSWh%.
- If VarName is absent,the outcome of the calculation with HASH will be displayed in a
  message window and stored in the clipboard.

