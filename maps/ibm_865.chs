;
; This file is a charset conversion module in text form.
;
; This module converts CP437 to CP865.
;
; Format: ID, version, level,
;         from charset, to charset,
;         128 entries: first & second byte
;	  "END"
; Lines beginning with a ";" or a ";" after the entries are comments
;
; Unknown characters are mapped to the "?" character.
;
; cedilla = ,   ; dieresis = ..       ; acute = '
; grave = `     ; circumflex = ^      ; ring = o
; tilde = ~     ; caron = v
; All of these are above the character, apart from the cedilla which is below.
;
; \ is the escape character: \0 means decimal zero,
; \dnnn where nnn is a decimal number is the ordinal value of the character
; \xnn where nn is a hexadecimal number
; e.g.: \d32 is the ASCII space character
; Two \\ is the character "\" itself.
;
0		; ID number
0		; version number
;
2		; level number
;
CP437		; from set
CP865           ;
;
\0 \x80		; C with cedilla
\0 \x81		; u dieresis
\0 \x82		; e acute
\0 \x83		; a circumflex
\0 \x84		; a dieresis
\0 \x85 	; a grave
\0 \x86		; a ring
\0 \x87		; c cedilla
\0 \x88		; e circumflex
\0 \x89		; e dieresis
\0 \x8A		; e grave
\0 \x8B		; i dieresis
\0 \x8C		; i circumflex
\0 \x8D		; i grave
\0 \x8E		; A dieresis
\0 \x8F		; A ring
\0 \x90		; E acute
\0 \x91		; ae
\0 \x92		; AE
\0 \x93		; o circumflex
\0 \x94		; o dieresis
\0 \x95		; o grave
\0 \x96		; u circumflex
\0 \x97		; u grave
\0 \x98		; y dieresis
\0 \x99		; O dieresis
\0 \x9A		; U dieresis
\0 c		; cent
\0 \x9C		; pound sterling
\0 Y		; yen
\0 \x9E		; x (multiply sign?)
\0 \x9F		; florin
\0 \xA0		; a acute
\0 \xA1		; i acute
\0 \xA2		; o acute
\0 \xA3		; u acute
\0 \xA4		; n tilde
\0 \xA5		; N tilde
\0 \xA6		; ord feminine
\0 \xA7		; ord masculine
\0 \xA8		; question downwards
\0 \xA9		;
\0 \xAA		; logical not
\0 \xAB		; half fraction
\0 \xAC		; quarter fraction
\0 \xAD		; exclam downwards
\0 \xAE		; guillemot left
> >		; guillemot right
\0 \xB0		;
\0 \xB1		;
\0 \xB2		;
\0 \xB3		;
\0 \xB4		;
\0 \xB5		;
\0 \xB6		;
\0 \xB7		;
\0 \xB8		;
\0 \xB9		;
\0 \xBA		;
\0 \xBB		;
\0 \xBC		;
\0 \xBD		;
\0 \xBE		;
\0 \xBF		;
\0 \xC0		;
\0 \xC1		;
\0 \xC2		;
\0 \xC3		;
\0 \xC4		;
\0 \xC5		;
\0 \xC6		;
\0 \xC7		;
\0 \xC8		;
\0 \xC9		;
\0 \xCA		;
\0 \xCB		;
\0 \xCC		;
\0 \xCD		;
\0 \xCE		;
\0 \xCF
\0 \xD0		;
\0 \xD1		;
\0 \xD2		;
\0 \xD3         ;
\0 \xD4		;
\0 \xD5		;
\0 \xD6		;
\0 \xD7		;
\0 \xD8		;
\0 \xD9		;
\0 \xDA		;
\0 \xDB		;
\0 \xDC		;
\0 \xDD		;
\0 \xDE		;
\0 \xDF		;
\0 \xE0		; O acute
\0 \xE1           ; german double s (misused as Beta)
\0 \xE2
\0 \xE3
\0 \xE4 		;
\0 \xE5		;
\0 \xE6		; mu
\0 \xE7
\0 \xE8
\0 \xE9
\0 \xEA	        ; Omega
\0 \xEB         ; delta
\0 \xEC
\0 \xED         ; o slash
\0 \xEE
\0 \xEF
\0 \xF0		; equivalent
\0 \xF1		; plusminus
\0 \xF2		; greater equals
\0 \xF3		; smaller equals
\0 \xF4
\0 \xF5
\0 \xF6         ; divide
\0 \xF7		; approx
\0 \xF8		; ring / degree
\0 \xF9            ; centered dot
\0 \xFA		; em dash
\0 \xFB		; radical
\0 \xFC		; to the nth power
\0 \xFD		; to the second power
\0 \xFE		;
\0 \xFF		; space
END
