; interrupts and constants for Sprain VM

INT_Quit = $00

; args
INT_Argc = $08  ; r1 <- num of args (at least 1)
INT_Argv = $09  ; r1 -> arg index  r2 -> max size  r3 -> dest
                ; fail:    r1 <- -1  r2 <- len
                ; success: r1 <- 0  r2 <- len

; file interrupts
INT_FOpen = $10       ; r1 -> mode  r2 -> filename (null-terminated string)
                      ; success: r1 <- 0  fail: r1 <- -1
INT_FClose     = $11
INT_FWriteChar = $12  ; r1 -> char
INT_FWrite     = $13  ; r1 -> size  r2 -> src
INT_FReadChar  = $14  ; r1 <- char
INT_FRead = $15       ; r1 -> max size  r2 -> dest
                      ; r1 <- total
INT_FEOF = $16        ; not eof: r1 <- 0  eof: r1 <- -1

; file modes
FILE_R = 0
FILE_W = 1

; console io
INT_PutChar = $20  ; r1 -> char
INT_GetChar = $21  ; r1 <- char
INT_KeyDown = $22  ; r1 <- -1/0 (true/false)
INT_Cursor  = $24  ; r2 -> y  r3 -> w
INT_Window  = $25  ; r2 <- h  r3 <- w
INT_Clear   = $26

; keys
K_UP    = $81
K_DOWN  = $82
K_RIGHT = $83
K_LEFT  = $84

; timing
INT_Ticks = $30  ; r1 <- ticks

INT_ToggleDebug = $69

