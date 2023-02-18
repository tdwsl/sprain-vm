\ generate fakefs.js

0 value fd-in
0 value fd-out

1024 64 * constant bufsz
create buf bufsz allot
create hbuf 2 allot

: hexc
  dup 10 < if [char] 0 + else [char] a + 10 - then ;

: write-hex
  s" 0x" fd-out write-file throw
  dup 16 / hexc hbuf c!
  15 and hexc hbuf 1+ c!
  hbuf 2 fd-out write-file throw ;

: write-name
  [char] " buf c! dup buf + 1+ [char] " swap c!
  2dup buf 1+ swap move
  buf over 2 + fd-out write-file throw ;

: add-file
  write-name
  r/o bin open-file throw to fd-in
  buf bufsz fd-in read-file throw
  fd-in close-file throw
  s" , [" fd-out write-line throw
  dup 0 do
    buf i + c@ write-hex
    s" ," fd-out i 1+ 12 mod 0= if
      write-line
    else write-file then
    throw
  loop
  12 mod if 0 0 fd-out write-line then
  s" ], " fd-out write-line throw ;

s" fakefs.js" w/o create-file throw to fd-out
s" var fs = [" fd-out write-line throw

s" test0.bin" add-file
s" test1.bin" add-file
s" test2.bin" add-file

s" ]; " fd-out write-line throw
s" function file(fname) {" fd-out write-line throw
s" for(var i = 0; i < fs.length/2; i++)" fd-out write-line throw
s" if(fs[i*2] == fname) return fs[i*2+1];" fd-out write-line throw
s" return null;" fd-out write-line throw
s" }" fd-out write-line throw
fd-out close-file throw

bye
