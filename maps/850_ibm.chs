;
; This file is a charset conversion module in text form.
;
; This module Converts IBM extended characters (codepage 850)
; to IBM extended characters (international codepage 437).
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
CP850		; from set
IBMPC           ;
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
\0 o		; o "slash"
\0 \x9C		; pound sterling
\0 O		; O "slash"
\0 x		; x (multiply sign?)
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
\0 R		; registered trademark
\0 \xAA		; logical not
\0 \xAB		; half fraction
\0 \xAC		; quarter fraction
\0 \xAD		; exclam downwards
\0 \xAE		; guillemot left
\0 \xAF		; guillemot right
\0 \xB0		;
\0 \xB1		;
\0 \xB2		;
\0 \xB3		;
\0 \xB4		;
\0 A		; A acute
\0 A		; A circonflex
\0 A		; A grave
\0 C		; copyright
\0 \xB9		;
\0 \xBA		;
\0 \xBB		;
\0 \xBC		;
\0 \x9B		; cent
\0 \x9D		; Yen
\0 \xBF		;
\0 \xC0		;
\0 \xC1		;
\0 \xC2		;
\0 \xC3		;
\0 \xC4		;
\0 \xC5		;
\0 a		; a tilde
\0 A		; A tilde
\0 \xC8		;
\0 \xC9		;
\0 \xCA		;
\0 \xCB		;
\0 \xCC		;
\0 \xCD		;
\0 \xCE		;
C u		; curency sign
\0 d		; islandic eth
\0 D		; islandic Eth
\0 E		; E circonflex
E e		; E dieresis
\0 E		; E grave
\0 i		; i without dot
\0 I		; I acute
\0 I		; I circonflex
\0 I		; I dieresis
\0 \xD9		;
\0 \xDA		;
\0 \xDB		;
\0 \xDC		;
\0 |		;
\0 I		; I grave
\0 \xDF		;
\0 O		; O acute
\0 \xE1         ; german double s (misused as Beta)
\0 O		; O circonflex
\0 O		; O grave
\0 o		; o tilde
\0 o		; O tilde
\0 \xE6		; mu
T h     	; icelandic Thorn
t h		; icelandic thorn
\0 U		; U acute
\0 U		; U circonflex
\0 U		; U grave
\0 y		; y acute
\0 Y		; Y acute
\0 -		; em dash
\0 '		; acute accent
\0 -		; soft hyphen
\0 \xF1		; plusminus
= =		; double arrow
\x1 ?		; three quaters
P i		; pilcrow sign, abused as Pi
\0 S		; paragraph
\0 \xF6		; divide
\0 ,		; ogonek
\0 \xF8		; ring / degree
\0 "		; dieresis
\0 .		; dot above
^ 1		; to the first power
^ 3		; to the third power
\0 \xFD		; to the second power
\0 \xFE		;
\0 \xFF		; space
END
