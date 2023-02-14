; divide some numbers

  org $1000-4
  dw $1000

  include "interrupts.inc"

  mov r1,10
  mov r2,-1
  blt r1,r2 +

  mov r3,vals
  - mov r1,(r3)
    beq r1,zero +
    add r3,4
    mov r2,(r3)
    add r3,4
    call printDiv
    bra -
  +

  int INT_Quit
  
vals:
  dw 20,5, 10,3, 100,9, 49,5, 0

printDiv: ; print r1/r2
  push r1
  push r2
  push r2
  call printNum
  mov r2,msg0
  call printStr
  pop r1
  call printNum
  mov r2,msg1
  call printStr
  pop r2
  pop r1
  call div
  push r2
  call printNum
  mov r2,msg2
  call printStr
  pop r1
  call printNum
  mov r1,10
  int INT_PrintChar
  ret
msg0: db " / ",0
msg1: db 9,"=   ",0
msg2: db 9,"r ",0

div: ; div r1 by r2
  push r3
  mov r3,zero
  - blt r1,r2 +
    sub r1,r2
    add r3,1
    bra -
  +
  mov r2,r1
  mov r1,r3
  pop r3
  ret

printNum: ; print r1
  push r2
  push r3
  mov r3,zero
  - mov r2,10
    call div
    push r2
    add r3,1
    bne r1,zero -

  - pop r1
    add r1,'0'
    int INT_PrintChar
    add r3,-1
    bne r3,zero -
  pop r3
  pop r2
  ret

printStr: ; print str at r2
  push r1
  mov r1,zero
  - mov l1,(r2)
    beq r1,zero +
    int INT_PrintChar
    add r2,1
    bra -
+ pop r1
  ret

