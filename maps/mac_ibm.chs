;
; This file is a charset conversion module in text form.
;
; This module Converts Macintosh extended characters to IBM-PC characters.
;
; Format: ID, version, level,
;         from charset, to charset,
;         128 entries: first & second byte
;	  "END"
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
MAC		; from set
CP437		; to set
;
\0 \x8E		; A dieresis
\0 \x8F		; A ring
\0 \x80		; C cedilla
\0 \x90		; E acute
\0 \xA5		; N tilde
\0 \x94		; O dieresis
\0 \x9A		; U dieresis
\0 \xA0		; a acute
\0 \x85		; a grave
\0 \x83		; a circumflex
\0 \x84		; a dieresis
\0 a		; a tilde
\0 \x86		; a ring
\0 \x87		; c cedilla
\0 \x82		; e acute
\0 \x8A		; e grave
\0 \x88		; e circumflex
\0 \x89		; e dieresis
\0 \xA1		; i acute
\0 \x8D		; i grave
\0 \x8C		; i circumflex
\0 \x8B		; i dieresis
\0 \xA4		; n tilde
\0 \xA2		; o acute
\0 \x95		; o grave
\0 \x96		; o circumflex
\0 \x94		; o dieresis
\0 o		; o tilde
\0 \xA3		; u acute
\0 \x97		; u grave
\0 \x96		; u circumflex
\0 \x81		; u dieresis
\0 +		; dagger
\0 \xF8		; ring / degree
\0 \x9B		; cent
\0 \x9C		; pound sterling
\0 \x15		; section
\0 \xFE		; bullet
\0 \x14		; paragraph
\0 \xE1		; german double s
\0 R		; registered trademark
\0 c		; copyright
T M		; trademark
\0 '		; acute
\x1 ?		; dieresis
< >		; not equal
\0 \x92		; AE
\0 0		; O slash
\0 \xEC		; infinity
\0 \xF1		; plusminus
\0 \xF3		; smaller equals
\0 \xF2		; greater equals
\0 \x9D		; Yen
\0 \xE6		; mu
\0 \xEB		; delta
\0 \xE4		; Sigma (summation)
\x1 ?		; Pi
\0 \xE3		; pi
\x1 ?		; integral
\0 \xA6		; ord feminine
\0 \xA7		; ord masculine
\0 \xEA		; Omega
\0 \x91		; ae
\0 \xED		; o slash
\0 \xA8		; question downwards
\0 \xAD		; exclam downwards
\0 \xAA		; logical not
\0 \xFB		; radical
\0 \x9F		; florin
\0 \xF7		; approx.
\0 \x7F		; Delta
\0 \xAF		; guillemot right
\0 \xAE		; guillemot left
. .		; ellipsis
\0 \d32		; non breaking space
\0 A		; A acute
\0 A		; A tilde
\0 O		; O tilde
O E		; OE
o e		; oe
\0 \xFA		; en dash
\0 -		; em dash
\0 "		; double quote left
\0 "		; double quote right
\0 `		; quote left
\0 '		; quote right
\0 \xF6		; divide
\0 \x04		; lozenge
\0 \x98		; y dieresis
\0 ?		;
\0 ?		;
E U		; Euro
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
\0 ?		;
END
