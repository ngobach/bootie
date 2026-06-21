# TEMP

To clear user's temporary folder or reset the position of user's temporary folder.
$:to set system variables as well.         A directory cannot be contained at the end.

## Syntax

```text
TEMP [[@]Delete|[$]Setting]  [InitDir][,VarName]
TEMP @[$]Setting   NewTempDir,[VarName]                  //to set in silence.
TEMP [*del] [*tmpl:[Front]*[Back]] *tmpdir [,]VarName    //to create the unique temporary
directory, *del:to automatically delete on exit.
TEMP [*del] [*tmpl:...]*tmpfile  [,]VarName[,DirVarName] //to create the unique temporary
file.
```

## Parameters

- **Delete**: to clear the temporary directory,@ indicates direct clearing without user's
  confirmation; Setting indicates resetting the position of the temporary directory.
- **VarName**: to return the old or newly-set temporary directory.

## Examples

```text
TEMP Delete  or  TEMP *del *tmpl:d:\A\B\CC*.txt *tmpfile  ,&&FName,&&Dir
```

## Remarks

- Do not use this commmand in a config file.It is used only after the desktop is loaded and
  it reads the position of the temporary directory from the registry.

