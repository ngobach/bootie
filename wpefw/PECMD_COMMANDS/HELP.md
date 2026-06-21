# HELP

To display the information of help.With *,CMDLineH is <=-100000 or null,and no command line.
If CMDLineH is >=100000,there will be no help window.

## Syntax

```text
HELP [-hlpdoc=HelpPage] [*[CMDLineH]] [[~]DirBkMark] [TxtForeColor][#TxtBkColor]
```

## Parameters

- **DirBkMark**: the directory entry to directly display(the first few characters); ~ indicates
  being forced to search for the text instead of bookmark.Being followed by . indicates that
  the current line is just displayed as the top line. * indicates a line break.
- **Num**: (Hex supported)

## Examples

```text
HELP 0x00EEFF#0xFF0000  or  HELP KeyVal or HELP  ~example│EDIT.
```

## Remarks

- Directly executing this command will also display the information of help without using
  command line params.

