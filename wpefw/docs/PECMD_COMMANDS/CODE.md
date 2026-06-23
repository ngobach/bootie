# CODE

To transform code.

## Syntax

```text
CODE [*nobom] [*transonly] [*[*]]-srcFmt,srcfile,[*[*]]-dstFmt,dstFile
CODE *jm   PeVar,+|-|^|#|>|.,Num_or_SrcOff,UniB,Off,Bytes[,StepS,Loops[,StepD]] //^Xor#Swap
 >RLopMov.Set&And|Or?RetNm[±Of]:BitQuery ΣRetNm:SUM ΠRetNm:product ΩRNm:QCount UnitB:1/2/4/8
```

## Parameters

- ***nobom**: not to import BOM; *transonly:to transform between -GBK and -BIG5.Not transform but
  only translate;
- **srcFmt**: source format; -dstFmt:destination format,including -UNICODE -UNI -UNICODEB -UNIB
  -ANSI -UTF8 -UTF7 -GBK -BIG5 -number-.- is a format of natural display and the number is a
  standard code ID. */*** before -srcFmt indicates srcfile is an hex string/PEVar to transf-
  orm; *- indicates the spaces at the front and back of the srcfile will be transformed;
  * before -dstFmt indicates transforming into an hex string and storing the outcome in a
  variable; dstFile is compatible with PUTF/GETF. No * indicates a file name; Some special
  characters will be transfered to a variable first before being referenced later,and its
  input can be of decimal;
  With *,- can be absent; For source format,select -BOM,it will be automatically judged
  according to BOM;
  ** before -srcFmt and -dstFmt indicates the content behind is a name of PE variable.

## Examples

```text
CODE  -ANSI,src.txt,-UNICODE,dst.txt        CODE **-GBK,&V1,**-UNI,&DST
CODE  *,NiHao,*UNICODE,UNI_HEX_NIHAO        CODE *,NiHaoMa,**-GBK,&DST
```

