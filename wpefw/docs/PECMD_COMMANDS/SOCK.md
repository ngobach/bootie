# SOCK

To define an automatically recyled object of network connection.--exist:check(PIPE may 231)
 --only:only one

## Syntax

```text
SOCK [*] ButtonName; [ProFamily][; ProType][; ProID] //Commas can be used,the same below.
SOCK --file [*] CtrName;[we][-rwd];FileName  //to open an automatically released file
     handle for API, w:writable we:writable+exist -:reset share
SOCK --shm [*] CtrName;[w];ShareName;Len[;Src;Off;InitV;InitN] //Alloc shared mem.w:writable
SOCK --event [*] CtrName;ShareName[;initv;bManual]        //Alloc event
SOCK --sem[aphore] [*] CtrName;ShareName[;initv;maxv]       //Alloc semaphore
SOCK --mutex [*] CtrName;ShareName[;InitLock(0)]          //Alloc Mutex
SOCK --mailslot [*] CtrName;ShareNam[;bServer;TimeoutMs]  //mailslot .read/write receve/send
SOCK --pipe [*] CtrName;ShareNam[;TimeoutMs;BufSZ;Link]   //pipe .read/write  .connect
           Link: 0x1:Now  0x2:Client 0x4:Server 0:Auto(First Server)
SOCK --gethostbyname[*|#]    IPName;HostName  //to return IP address,*:multiple addresses
     are separated by space,#:integer.
```

## Parameters

  * before <ButtonName> indicates being automatically recycled before quitting the block or
  function.
- **ButtonName**: string,should be unique and not the same as those of other controls or
  environment variables.
- **ProFamily**: generally,the default internetwork:AF_INET=2.
- **ProType**: with connection:SOCK_STREAM=1(default),without connection:SOCK_DGRAM=2
- **ProID**: TCP:IPPROTO_TCP=6(default) UDP:IPPROTO_UDP=17   IP:IPPROTO_IP=0

## Examples

```text
SOCK s1 or SOCK s2; ; %&SOCK_DGRAM%; %&IPPROTO_UDP%
```

## Remarks

- "ENVI @Name.connect=[ErrVarName]; IP; PortID",to connect to the server.ErrVarName returns
  the error code,0:no error.
- "ENVI @Name.write=[ErrVarName]; [LenVarName]; [DataVarName]; [NumByteSent[@Offset]]
  [; SendMark][; IP][; PortID]".LenVarName returns the actual number of bytes sent,which is
  the number of bytes contained in DataVarName by default.If connected,"IP PortID" is not
  necessary or applicable to SOCK_STREAM.
- "ENVI @Name.read=[ErrVarName]; [LenVarName]; [DataVarName]; [*][NumByteReceived[@Offset]]
  [; SendMark][; IP][; PortID]".LenVarName returns the actual number of bytes received,
  which is the number of bytes contained in DataVarName by default.* indicates automatically
  trying reading for many times until all the data is read.
  "IPVarName PortIDVarName":to return the IP and port of the opposite side,with # before
  IPName,the command will return the object of address and the length of the object.
- "ENVI @Name.close=[ErrVarName]",to automatically release the connection.ErrVarName returns
  the error code,0:no error.
- "ENVI @Name.sock=[ErrVarName][;ProFamily][;ProType][;ProID]",to regenerate a new sock.
- "ENVI @Name.shutdown=[ErrVarName][;Mode]",to friendlily inform closing the connection.
  default(2):to close bidirectionally.0:to receive  1:to send
  [.sock] and [.close] contain this function.
- "ENVI @Name.bind=[ErrVarName]; IP; PortID",to bind the server with monitored IP and port.
- "ENVI @Name.listen=[ErrVarName][ConnPoolSize]",the number of the waiting connections to be
  simutaneously cached,7 by default.
- "ENVI @Name.accept=[ErrVarName];[MonitorConnfd][; IPVarName][; PortVarName]".MonitorConnfd
  is the father connection number of listen.
  "IPVarName PortIDVarName":to return the IP and port of the opposite side,with # before
  IPVarName,the command will return the object of address and the length of the object.
- "ENVI @Name.fd=fdVarName",fdVarName returns the internal connection number of the
  connection for accept,select and WINDOWSAPI.
- "ENVI @Name.mem=mVName",mVName returns the SHM address. ENVI-mkfixdummy &&V=%mVName%:len
- "ENVI @Name.setsockopt=[ErrVName];[CfgLevel];CfgItem;CfgDatVName[;CfsgDatsLen]"
  ConfigLevel is SOL_SOCKET(0xffff) by default.ConfigDataLen is all the data by default.
  Commonly used:TEAM ENVI#$ &&v=1| ENVI @sv.setsockopt=; ; %&SO_REUSEADDR%,&v //allowed to
  remonitor the address.
- "ENVI @Name.ioctlsocket=[ErrVName];;ConfigItem; ConfigDataVarName"
- "ENVI @Name.DeviceIoControl=[ErrVName];OP;IAddr|*PENm;InLen;OAddr|*PeNm;Olen;RNM[;lpov]
- "ENVI @Name.select=[ErrVName]; MSTimeout; [[ReadRetVarName:]readfd1:readfd2:...]
  [; [WriteRetVarName:]writefd1:writefd2:...][; [AbnormalRetVarName:]abnormalfd1:
  abnormalfd2:..]. "," indicates itself.
  If params from ReadRetVarName to the end all absent,the default is to wait till the socket
  is ready for data reading.The default of MSTimeout is -1 indicating waiting all the time
  and 0 indicating checking and returning immediately.
- "ENVI @Name.getname=[ErrVarName]; [0/1]; [IPVarName][; PortIDVarName]".to return IP and
  port of this side(0 by default)/the opposite side(1).The same as above.
- "ENVI @Name.wait=[ErrVName][;ms][;[[*]handle2:handle3...]]"  //wait .def INFINITE  *:All
  "ENVI @@wait=(Type)ShareName;[ErrVName][;ms][;[[*]handle2:handle3...]]"
- "ENVI @Name.setevent=[ErrVName][;1][;NmOldV]"  //set/free it. 0 for clear all event
  "ENVI @@setevent=(Type)ShareName;[ErrVName][;1][;NmOldV]"   Type:event/mutex/sem[aphore]

