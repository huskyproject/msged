;
; This file is a charset conversion module in text form.
;
; This module converts CP865 to CP866.
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
CP865		; from set
CP866		; to set
;
\0 C		; C with cedilla
u e		; u dieresis
\0 e		; e acute
\0 a		; a circumflex
a e		; a dieresis
\0 a		; a grave
\0 a		; a ring
\0 c		; c cedilla
\0 e		; e circumflex
e e		; e dieresis
\0 e		; e grave
i e		; i dieresis
\0 i		; i circumflex
\0 i		; i grave
A e		; A dieresis
\0 A		; A ring
\0 E		; E acute
a e		; ae
A E		; AE
\0 o		; o circumflex
o e		; o dieresis
\0 o		; o grave
\0 u		; u circumflex
\0 u		; u grave
y e		; y dieresis
O e		; O dieresis
U e		; U dieresis
\0 o		; o stroke
\0 #		; pound sterling
\0 O		; O stroke
P t		; Pt
\0 f		; florin
\0 a		; a acute
\0 i		; i acute
\0 o		; o acute
\0 u		; u acute
\0 n		; n tilde
\0 N		; N tilde
\0 a		; ord feminine
\0 o		; ord masculine
\0 ?		; question downwards
\0 -		; 
\0 !		; logical not
. 5		; half fraction
\x1 ?		; quarter fraction
\0 !		; exclam downwards
< <		; guillemot left
C u		; currency sign
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
\0 \xCF		;
\0 \xD0		;
\0 \xD1		;
\0 \xD2		;
\0 \xD3		;
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
\0 a		; alpha
s s             ; german double s (misused as Beta)
\x1 ?		; Gamma
p i		; pi
\x1 ?		; Sigma (summation)
\x1 ?		; sigma
m u		; mu
\x1 ?		; tau
\x1 ?		; Phi
\x1 ?		; Theta
\0 O		; Omega
\0 d		; delta
\x1 ?		; infinity
\0 o		; o slash
\x1 ?		; element
\x1 ?		; intersection
= =		; equivalence
+ -		; plusminus
> =		; greater equals
< =		; smaller equals
\x1 ?		; integral top
\x1 ?		; integral bottom
\0 /		; divide
~ =		; approx.
\0 \xF8		; ring / degree
\0 \xF9		; centered dot
\0 \xFA		; en dash
\0 \xFB		; radical
^ n		; to the n'th power
^ 2		; to the second power
\0 \xFE		;
\0 \xFF		; space
END
