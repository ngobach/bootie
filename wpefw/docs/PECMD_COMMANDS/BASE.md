# BASE

Auxiliary command of ADSL to calculate the BASE64 variation code of a string,which is used
for such data as the encrypted user's password of ADSL.

## Syntax

```text
BASE       <Str>[,VarName]
BASE* [-u] <Str>[,VarName]
```

## Parameters

- **Str**: The string used for calculating the BASE64 variation code,supportive of a mixed string
  of Chinese and English,case sensitive.
- **VarName**: to store the outcome of calculation.

## Examples

```text
BASE mdyblog@163.com,AdslUserName or BASE PassWord,AdslPassWord
```

## Remarks

- When the BASE64 variation code is calculated by BASE,the number of the characters in the
  string should be limited within 256(containing no ",").
- VarName given,BASE will store the outcome of calculation in the specified variable.
  Example 2 can be referenced via the environment variable %AdslPassWord%.
- If VarName is absent,BASE will display the outcome of calculation in the form of a message
  window and meanwhile store them in the clipboard.
- BASE adopts BASE64 variation algorithm,the outcome of which differs from that of a common
  algorithm to ensure the safety of user's data,thus it offers no decoding command as well.
- BASE* refers to a common algorithm,offering decoding command.

