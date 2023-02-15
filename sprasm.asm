; native assembler for sprain vm

  org $1004-4
  dw $1004

  include "interrupts.inc"

  ;int INT_ToggleDebug
  int INT_Argc
  add r1,-3
  beq r1,zero +
    mov r2,usageMsg
    call printStr
    int INT_Quit
  +

  mov r1,1
  mov r2,filenameBufSz
  mov r3,filenameBuf
  int INT_Argv
  call asmFile

  int INT_Quit

usageMsg: db "usage: sprasm <file> <out>",10,0
failROpenMsg: db "failed to open input file",10,0
failWOpenMsg: db "failed to open output file",10,0

asmFile:
  mov r1,FILE_R
  mov r2,filenameBuf
  int INT_FOpen
  beq r1,zero +
    mov r2,failROpenMsg
    call printStr
    int INT_Quit
  +

  mov r2,lineBuf
  mov r1,1
  mov (r2),r1

  mov r2,lineBuf
  - int INT_FReadChar
    mov (r2),l1
    int INT_FEOF
    beq r1,zero +
      mov r1,10
      mov (r2),l1
    +
    mov l1,(r2)
    add r1,-10 ; newline
    bne r1,zero +
      mov (r2),lzero
      call asmLine
      mov r2,lineNumber
      mov r1,(r2)
      add r1,1
      mov (r2),r1
      mov r2,lineBuf
      int INT_FEOF
      bne r1,zero asmFileEOF
      bra -
    +
    add r2,1
    bra -

asmFileEOF:
  int INT_FClose
  ret

asmLine:
  mov r2,lineBuf
  call printStr
  mov r1,10
  int INT_PrintChar
  ret

printStr:
  mov r1,zero
  - mov l1,(r2)
    beq r1,zero +
    int INT_PrintChar
    add r2,1
    bra -
  +
  ret

; reserve memory

filenameBufSz = 100
filenameBuf: allot filenameBufSz
lineNumber: allot 4
lineBufSz = 200
lineBuf: allot lineBufSz
orgv: allot 4
addr: allot 4
memory:
