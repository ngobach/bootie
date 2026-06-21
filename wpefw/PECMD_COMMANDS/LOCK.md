# LOCK

To apply for mutiple automatic genius locks in the stack of the current function and get all
the specified ones locked/released at the same time.

## Syntax

```text
LOCK  [--exist|--try[*]] [-backlevel:][#[#]]Lock1Name Lock2Name Lock3Name... [,RetVarName]
```

## Parameters

- **LockName**: a string of normal letters,figures or Chinese characters.# indicates process lock
  among processes; ## indicates system lock between different terminal services; Otherwise,
  it refers to the thread lock inside the process.Not case sensitive.Generally,different
  login users can use #%USERNAME%#subname,as well as GUID,like
  {5ACCA58C-8EAB-410E-9263-C5ADAD0D8F9B}.
- **backlevel**: a positive number,0 by default.It indicates no destruction after exit but minus
  one and being upgraded one level,used in the subprocess to create a lock and calculated
  according to the minimum number from head here.
  --exist:to check if there exists such a lock.0:not exist,1:exists,-1:failed.It mainly aims
  to avoid many times of locking.
  --try:try to lock.0:locked successfully,1:failed to lock.If locked,an operation of void
  locking by using --try* is practicable.

## Examples

```text
LOCK  SellTrainTicket  QueryTrainTicket or LOCK --exist #TrainTicketSellSys or LOCK .thread
      or LOCK .ole
```

## Remarks

- Programmers do not need to unlock.Unlocking occurs when the function returns,the window
  destructed or the program quits the block.It will not be easy to give rise to interlocking
  if the multiple locks are all locked together at the same time.Repeated locking is
  permitted.It is a good habbit to put the command at the very head of a function or in the
  same line closely following {. .thread indicates multi-thread synchronization;
  .ole indicates OLE synchronization.
  An appointment:#pecmd indicates the common instant lock to atomize a set of operations.

