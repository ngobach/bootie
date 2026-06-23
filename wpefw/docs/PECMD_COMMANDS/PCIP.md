# PCIP

To set the IP address for the computer.Format 1 for static IP(all param cannot be absent);
Format 2 for dynamic IP(DNS can be specified).
☼PICP should be closely followed by params: *safe:conserved version; *qk:version quicker
  than NT5.x normal OS; *qkos:quick version(default).The three versions should be used in
  coordination. *fresh:to refresh by force(default),*nfresh:not to refresh.*sfsw:to switch
  from NT5.x DHCP to static IP,reliable but slower.*all:all netcards,otherwise ethernet and
  wireless net.*cah: &CacheName

## Syntax

```text
PCIP [IPAddr],[SubnetMask],[DeftGtway],[DNSAddr],[NetcardSN|NetcardName][,NetcardStat]
                                                                             //static IP
PCIP -,[-],[-],[DNSAddr],[NetcardSN|NetcardName][,NetcardStat]               //dynamic IP
PCIP ?[*][IPName],[SubnetName],[GtwayName],[DNSName],[NetcardSN][?][NetNameName][,MACName]
     [,LinkName][,DHCPName][,bDHCP][,Status][,Media][,Desc][,Type] //to query. *:Real val
```

## Parameters

- **IPAddr**: string in the form of IP indicating the IP address of net,being absent indicates
  not modifying the IP address,"-" indicates automatically obtaining the IP address;
- **SubnetMask**: string in the form of IP indicating the mask for subnet.Being absent indicates
  not modifying the subnet mask.IP being "-" indicates automatically obtaining the subnet
  mask.Multi-IP and multi-mask are supported,separated by "; ",":" or "&".Multi-IP should
  be corresponding to multi-mask.See examples.
- **DeftGtway**: string in the form of IP indicates the gateway address.Being absent indicates
  not modifying it.IP being "-" indicates automatically obtaining the gateway address.
- **DNSAddr**: string in the form of IP indicates the DNS address.The preferential DNS and spare
  DNS are linked with"; ", "&" or ":".Being absent indicates not modifying the DNS address.
  "-" means automatically obtaining the address of the DNS server. ',' should be transfered
  to a variable.If IP,SubnetMask,DeftGtway and DNS are 0.0.0.0,it indicates 0 clearing,used
  in the first setting.
- **NetcardSN**: number,can be absent,default 0 means the first netcard,1 the second,2 the third
  and the like;
- **NetcardName**: to be queried beneath {D8D9826A-332E-4858-9306-374EC20A9F05}
  HKLM\SYSTEM\CurrentControlSet\Control\Network\{4D36E972-E325-11CE-BFC1-08002BE10318},
  It also can be netcard description like Intel 8255x-based PCI Ethernet Adapter (10/100),
  MAC address,like 00-00-3D-46-34-D7 or 00:00:3D:46:34:D7 and "=LinkName" like =local link.
- **NetcardStat**: number,used for operating the serviceability of the netcard.0:disabled,
  1:enabled,fresh:disabled and enabled.Absent,no operation to it.

## Examples

```text
PCIP 192.168.1.2,255.255.255.0,192.168.1.1,192.168.1.1 or PCIP -,-,-,-,,
PCIP ,,,192.168.1.1;10.2.22.1,,1 or PCIP ,,,,,1 //to enable the netcard or
PCIP ,,,,,0                                     //to disable the netcard.
PCIP ? IP,MASK,GW,DNS,0?NAME,MAC,LINK   //multi IP multi DNS,separated by space or
PCIP *fresh                                     //to refresh.
PCIP 192.168.1.2;192.168.2.2,255.255.255.0;255.255.255.0,192.168.1.1,192.168.1.1 //multi IP
PCIP 192.168.1.2,255.255.255.0,0.0.0.0,0.0.0.0  //set GW,DNS as 0.0.0.0 anyway.
```

## Remarks

- This command may be ineffective if the netcard driver is not installed,the netcard is
  disabled or the OS does not support the network;
- This command contains many params,as are usually applied in example 1(no enabling
  operation when IP for the first netcard is set) or in example 2(to automatically obtain IP
  and DNS).

