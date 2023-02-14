  org $00001000-4
  dw $00001000

  include "interrupts.inc"

  mov r2,helloMsg
  call printStr
  int INT_Quit

helloMsg:
  db "Hello, world! ",10,0

printStr:
  mov r1,zero
  - mov l1,(r2)
    beq r1,zero +
    int INT_PrintChar
    add r2,1
    bra -
+ ret
