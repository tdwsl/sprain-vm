  org $00001000-4
  dw $00001000

  mov r2,helloMsg
  call printStr
  int 0

helloMsg:
  db "Hello, world! ",10,0

printStr:
  mov r1,zero
  - mol r1,(r2)
    beq r1,zero +
    int 2
    add r2,1
    bra -
+ ret
