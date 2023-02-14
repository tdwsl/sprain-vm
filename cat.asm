; print file

  org $1000-4
  dw $1000

  include "interrupts.inc"

  ;int INT_ToggleDebug
  int INT_Argc
  add r1,-2
  beq r1,zero +

  mov r2,errMsg1
  call printStr
  int INT_Quit

+ mov r1,1
  mov r2,filenameBufSz
  mov r3,filenameBuf
  int INT_Argv
  beq r1,zero +

  mov r2,errMsg2
  call printStr
  int INT_Quit

+ mov r1,FILE_R
  mov r2,filenameBuf
  int INT_FOpen
  beq r1,zero +

  mov r2,errMsg3
  call printStr
  int INT_Quit

+ call catFile
  int INT_FClose
  int INT_Quit   ; end

errMsg1:
  db "usage: cat.bin <file>",10,0
errMsg2:
  db "filename too long",10,0
errMsg3:
  db "failed to open file",10,0

catFile:
  - int INT_FReadChar
    mov r2,r1
    int INT_FEOF
    bne r1,zero +
    mov r1,r2
    int INT_PrintChar
    bra -
+ ret

printStr:
  mov r1,zero
  - mov l1,(r2)
    beq r1,zero +
    int INT_PrintChar
    add r2,1
    bra -
+ ret

filenameBuf:
filenameBufSz = 100
