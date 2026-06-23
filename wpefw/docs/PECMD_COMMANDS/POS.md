# POS POSITION SIZE LEVEL

## Parameters

  ENVI @WinAndCtrlName.POS=L:T:[@]W:H:Level:Transp:Front:Actv:PareWinID::CustW:CustH //@RealWH
  ENVI @WinAndCtrlName.POS=?[@]L_VarName:T_VarName:[@]W_VarName:H_VarName:X0_VarName:
  Y0_VarName:Actv_VarName:PareWinID_VarName:CustW_VarName:CustH_VarName        //to query
  ENVI @@POS=WinID:L:T:[@]W:H:Level:[$]Transp:Front:Actv:PareWinID::CustW:CustH
  ENVI @@POS=?WinID:[@]L_VarName:T_VarName:W_VarName:H_VarName:X0_VarName:Y0_VarName:
  Actv_VarName:PareWinID_VarName:CustW_VarName:CustH_VarName                        //to query
  Format1:to set,Level1:BLevel,2:CancTLevel,3:TLevel,4:FixedOnTLevel,Null or *:unchanged,
  Frnt1:at the front,Actv1:activated,$ before transparency refers to the format of 0-255,
  Format2:to query,(X0,Y0):the customer base relative coordinates,null is permitted,
  @L indicates returning screen absolute coordinates,@W returning the real width and height.

## Examples

```text
ENVI @WIN1.POS=%X1%:300:300:180:4  //to move,change its size and fix it on the top level.
 ENVI @B1.POS=?X2:Y2:W2:H2          //to query its size.
```

