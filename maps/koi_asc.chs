;
; This file is a charset conversion module in text form.
;
; This module converts KOI8-R cyrillics to 7 bit ASCII (transcription)
;
; Format: ID, version, level,
;         from charset, to charset,
;         128 entries: first & second byte
;         "END"
; Lines beginning with a ";" or a ";" after the entries are comments
;
; Unkown characters are mapped to the "?" character.
;
; cedilla = ,   ; dieresis = ..       ; acute = '
; grave = `     ; circumflex = ^      ; ring = o
; tilde = ~     ; caron = v
; All of these are above the character, apart from the cedilla which is below.
;
; \ is the escape character: \0 means decimal zero,
; \dnnn where nnn is a decimal number, is the ordinal value of the character
; \xnn where nn is a hexadecimal number
; e.g.: \d32 is the ASCII space character
; Two \\ is the character "\" itself.
;
0               ; ID number
0               ; version number
;
2               ; level number
;
KOI8-R          ; from set
ASCII           ; to set
;
\0 \d128        ; (missing) These codes are unused in the KOI8-R set.
\0 \d129        ; (missing) For transparency, these are not mapped.
\0 \d130        ; (missing)
\0 \d131        ; (missing) 4
\0 \d132        ; (missing)
\0 \d133        ; (missing)
\0 \d134        ; (missing)
\0 \d135        ; (missing) 8
\0 \d136        ; (missing)
\0 \d137        ; (missing)
\0 \d138        ; (missing)
\0 \d139        ; (missing) 12
\0 \d140        ; (missing)
\0 \d141        ; (missing)
\0 \d142        ; (missing)
\0 \d143        ; (missing) 16
\0 \d144        ; (missing)
\0 \d145        ; (missing)
\0 \d146        ; (missing)
\0 \d147        ; (missing) 20
\0 \d148        ; (missing)
\0 \d149        ; (missing)
\0 \d150        ; (missing)
\0 \d151        ; (missing) 24
\0 \d152        ; (missing)
\0 \d153        ; (missing)
\0 \d154        ; (missing)
\0 \d155        ; (missing) 28
\0 \d156        ; (missing)
\0 \d157        ; (missing)
\0 \d158        ; (missing)
\0 \d159        ; (missing) 32
\0 \x20         ; non-breaking space
Y o             ; Cyrillic_JO
\0 c            ; cent
Y o		; some koi fonts have jo here ...
\0 \x0f         ; currency
\0 Y            ; Yen
\0 |            ; broken bar
\0 \x15         ; section
\x1 ?           ; dieresis
y o             ; Cyrillic_jo
\0 a            ; ord feminine
< <             ; guillemot left
\0 !            ; logical not
\0 -            ; soft hyphen (or em dash)
\0 R            ; registered trademark
\x1 ?           ; overbar (macron)
\x1 ?           ; ring / degree
+ -             ; plusminus
^ 2             ; superscript two (squared)
y o		; some koi fonts have yo here ...
\0 '            ; acute
m u             ; mu
\0 \x14         ; paragraph
\0 o            ; bullet
\0 ,            ; cedilla
^ 1             ; superscript one
\0 o            ; ord masculine
> >             ; guillemot right
\x1 ?           ; one quarter
. 5             ; half
\x1 ?           ; three quarters
\0 ?            ; question downwards
y u
\0 a
\0 b
t s
\0 d
\0 e
\0 f
\0 g
\0 h
\0 i
\0 j
\0 k
\0 l
\0 m
\0 n
\0 o
\0 p
y a
\0 r
\0 s
\0 t
\0 u
z h
\0 w
\0 \x27
\0 y
\0 z
s h
\0 e
s z
c h
\0 \x60
Y u
\0 A
\0 B
T s
\0 D
\0 E
\0 F
\0 G
\0 H
\0 I
\0 J
\0 K
\0 L
\0 M
\0 N
\0 O
\0 P
Y a
\0 R
\0 S
\0 T
\0 U
Z h
\0 W
\0 \x27
\0 Y
\0 Z
S h
\0 E
S z
C h
\0 \x60
END
