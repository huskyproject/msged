;
; This file is a charset conversion module in text form.
;
; This module converts KOI8-R cyrillics to CP1125
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
CP1125          ; to set
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
\x1 ?
\x1 ?
\0 \xf1	; yo small
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\0 \xf2 ; yo capital
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\x1 ?
\0 \xEE
\0 \xA0
\0 \xA1
\0 \xE6
\0 \xA4
\0 \xA5
\0 \xE4
\0 \xA3
\0 \xE5
\0 \xA8
\0 \xA9
\0 \xAA
\0 \xAb
\0 \xAC
\0 \xAD
\0 \xAE
\0 \xAF
\0 \xEF
\0 \xE0
\0 \xE1
\0 \xE2
\0 \xE3
\0 \xA6
\0 \xA2
\0 \xEC
\0 \xEB
\0 \xA7
\0 \xE8
\0 \xED
\0 \xE9
\0 \xE7
\0 \xEA
\0 \x9E
\0 \x80
\0 \x81
\0 \x96
\0 \x84
\0 \x85
\0 \x94
\0 \x83
\0 \x95
\0 \x88
\0 \x89
\0 \x8A
\0 \x8b
\0 \x8C
\0 \x8D
\0 \x8E
\0 \x8F
\0 \x9F
\0 \x90
\0 \x91
\0 \x92
\0 \x93
\0 \x86
\0 \x82
\0 \x9C
\0 \x9B
\0 \x87
\0 \x98
\0 \x9D
\0 \x99
\0 \x97
\0 \x9A
END
