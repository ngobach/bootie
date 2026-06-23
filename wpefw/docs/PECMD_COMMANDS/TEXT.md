# TEXT

To display the text in the login sreen or desktop window.\n to break a line,and the suffix .
indicates not converting the newline in the raw mode.

## Syntax

```text
TEXT[.] [Txt][#Color][LLeft][TTop][RRight][BBottom][=+-][$FontSize[:FontName]][*]
```

## Parameters

- **If Txt is null,the text in the lately-defined rectangle area will be cleared.The default**
  color is white and the default coordinates is roughly at the top left corner. Ending with
  "*" indicates not clearing the originally displayed text before displaying the new.
  #LTRB=+- should be capitalized and the sequence cannot be changed. = indicates right
  aligned,+/- indcates horizontally/vertically center aligned.

## Examples

```text
TEXT Registering components……#0xFFDDDD L4 T720 R300 B768 $20
```

## Remarks

- This command displays a multiline text.
- The FontSize is 16 by default(equivalent to the typeface of Song,small number 5).
- The specified position[left,top,right,bottom] is related to the length of the text and
  size of font.
- This command will display the text on the login screen if it is used during login
  (in config file),and in the desktop window if it is used in Windows(after login).
  If params are absent,it will clear the last displayed text.If the text contains the
  characters #LTRB and they do not appear as params behind,they can be transfered to
  variables.

