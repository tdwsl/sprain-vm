; test console output

  org $100-4
  dw $100

  include "sprain.inc"

  int INT_Clear
  mov r1,'*'
  int INT_PutChar
  int INT_Window
  mov r4,screenH
  mov (r4),r2
  add r4,4
  mov (r4),r3
  sub r2,1
  sub r3,1
  mov r4,r2
  int INT_Cursor
  int INT_PutChar
  mov r2,zero
  int INT_Cursor
  int INT_PutChar
  mov r2,r4
  mov r3,zero
  int INT_Cursor
  int INT_PutChar

  mov r2,scroll
  mov (r2),zero

  mov r7,scroll
  - call draw
    call update
    call delay
    bra -

update:
  mov r2,scroll
  mov r3,screenW
  mov r3,(r3)
  mov r1,(r2)
  add r1,1
  blt r1,r3 +
    sub r1,r3
  +
  mov (r2),r1
  ret

delay:
  int INT_Ticks
  mov r3,100
  mov r2,r1
  - int INT_KeyDown
    bne r1,zero +
    int INT_Ticks
    sub r1,r3
    blt r1,r2 -
  ret
+ int INT_Quit

draw:
  mov r2,screenH
  mov r2,(r2)
  shr r2,1
  mov r3,zero
  int INT_Cursor

  mov r5,screenW
  mov r6,scroll
  mov r3,(r5)
  mov r1,' '
  - int INT_PutChar
    sub r3,1
    bne r3,zero -

  mov r3,(r6)
  mov r4,.msg
  call printStr

  mov r3,(r6)
  mov r5,(r5)
  add r3,r5
  mov r4,.msg
  call printStr

  mov r3,(r6)
  ;shr r5,1
  sub r3,r5
  mov r4,.msg
  call printStr

  mov r2,zero
  mov r3,zero
  int INT_Cursor

  ret

.msg: db "H  E  L  L  O     W  O  R  L  D",0

printStr: ; r2=y, r3=x, r4=msg
  - mov r1,zero
    mov l1,(r4)
    beq r1,zero .end

    call inBounds
    beq r1,zero +
    mov r1,zero
    mov l1,(r4)

    int INT_Cursor
    int INT_PutChar

  + add r3,1
    add r4,1
    bra -

.end
  ret

inBounds: ; r2,r3 in bounds?
  blt r2,zero .no
  blt r3,zero .no
  mov r1,screenH
  mov r1,(r1)
  bge r2,r1 .no
  mov r1,screenW
  mov r1,(r1)
  bge r3,r1 .no
  mov r1,zero
  sub r1,1
  ret
.no:
  mov r1,zero
  ret

screenH: allot 4
screenW: allot 4
scroll: allot 4
