; print file

  org $1000-4
  dw $1000

  include "sprain.inc"

  ;int INT_ToggleDebug
  int INT_Argc
  add r1,-2
  beq r1,zero +

  mov r2,errMsg1
  call printStr
  int INT_GetChar
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
  int INT_GetChar
  int INT_Quit

+ call catFile
  int INT_FClose
  int INT_GetChar
  int INT_Quit   ; end

errMsg1:
  db "usage: cat.bin <file>",10,0
errMsg2:
  db "filename too long",10,0
errMsg3:
  db "failed to open file",10,0

catFile:
  int INT_Window
  mov r4,r2
  mov r3,zero
  .l0:
    int INT_FReadChar
    mov r5,r1
    sub r5,10
    bne r5,zero +
      add r3,1
      bne r3,r4 +
      mov r3,zero
      int INT_GetChar
      int INT_Clear
      bra .l0
    +
    mov r2,r1
    int INT_FEOF
    bne r1,zero .end
    mov r1,r2
    int INT_PutChar
    bra .l0
.end:
  ret

printStr:
  mov r1,zero
  - mov l1,(r2)
    beq r1,zero +
    int INT_PutChar
    add r2,1
    bra -
+ ret

filenameBuf:
filenameBufSz = 100
