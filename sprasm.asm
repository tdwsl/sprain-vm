; native assembler for sprain vm (wip)

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

whiteStr: db 9," :,",0

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
      bne r1,zero .EOF
      bra -
    +
    add r2,1
    bra -

.EOF:
  int INT_FClose
  ret

asmLine:
  ; comment
  mov r2,lineBuf
  mov r1,';'
  call strchr
  beq r2,zero +
    mov (r2),lzero
  +

  ; tokenize
  mov r4,tokmem
  mov r5,r4
  mov r6,tokens
  mov r3,lineBuf
  - mov l1,(r3)
    beq r1,zero .white
    mov r2,whiteStr
    call strchr
    beq r2,zero .nowhite
    .white:
      beq r4,r5 +
        mov (r5),lzero
        mov (r6),r4
        add r6,4
        add r5,1
        mov r4,r5
      +
      beq r1,zero .end
      add r3,1
      bra -
    .nowhite:
    mov r2,r1
    add r2,-34 ; "
    bne r2,zero +
      ;;
    +
    mov (r5),l1
    add r5,1
    add r3,1
    bra -
.end:
  mov r2,lastToken
  mov (r2),r6

  mov r1,10
  int INT_PrintChar

  mov r1 tokens
  bne r1,r6 +
    ret
  +

  mov r3,tokens
  mov r4,lastToken
  mov r4,(r4)
  - mov r2,(r3)
    call printStr
    mov r1,10
    int INT_PrintChar
    add r3,4
    blt r3,r4 -

  ret

strcmp:
  mov r3,zero
  mov r4,zero
  - mov l3,(r1)
    mov l4,(r2)
    beq r3,r4 +
      mov r1,zero
      ret
    +
    beq r3,zero +
    add r1,1
    add r2,1
    bra -
+ ret

strchr:
  push r3
  mov r3,zero
  - mov l3,(r2)
    beq r3,zero .N
    bne r3,r1 +
      pop r3
      ret
    +
    add r2,1
    bra -
.N:
  mov r2,zero
  pop r3
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
lastToken: allot 4
tokens: allot 120
tokmem: allot 200
orgv: allot 4
addr: allot 4
memory:

