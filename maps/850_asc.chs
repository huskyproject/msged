;
; This file is a charset conversion module in text form.
;
; This module Converts IBM extended characters (codepage 850) to ASCII
; characters.
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
ASCII		; to set (change to UK for UK character set)
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
\0 o		; o "slash"
\0 #		; pound sterling
\0 O		; O "slash"
\0 x		; x (multiply sign?)
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
\0 R		; registered trademark
\0 !		; logical not
. 5		; half fraction
\x1 ?		; quarter fraction
\0 !		; exclam downwards
< <		; guillemot left
> >		; guillemot right
\0 #		;
\0 #		;
\0 #		;
\0 |		;
\0 |		;
\0 A		; A acute
\0 A		; A circonflex
\0 A		; A grave
\0 C		; copyright
\0 |		;
\0 |		;
\0 +		;
\0 +		;
\0 c		; cent
\0 Y		; Yen
\0 +		;
\0 +		;
\0 -		;
\0 -		;
\0 |		;
\0 -		;
\0 +		;
\0 a		; a tilde
\0 A		; A tilde
\0 +		;
\0 +		;
\0 =		;
\0 =		;
\0 |		;
\0 =		;
\0 +		;
C u		; currency sign
\0 d		; eth
\0 D		; Eth
\0 E		; E circonflex
E e		; E dieresis
\0 E		; E grave
E U		; Euro currency sign
\0 I		; I acute
\0 I		; I circonflex
\0 I		; I dieresis
\0 +		;
\0 +		;
\0 #		;
\0 n		;
\0 |		;
\0 I		; I grave
\0 ~		;
\0 O		; O acute
s s             ; german double s (misused as Beta)
\0 O		; O circonflex
\0 O		; O grave
\0 o		; o tilde
\0 o		; O tilde
m u		; mu
T h		; icelandic Thorn
t h		; icelandic thorn
\0 U		; U acute
\0 U		; U circonflex
\0 U		; U grave
\0 y		; y acute
\0 Y		; Y acute
\0 -		; em dash
\0 '		; acute accent
\0 -		; soft hyphen
+ -		; plusminus
= =		; double arrow
\x1 ?		; three quaters
P i		; pilcrow sign, abused as Pi
\0 S		; paragraph
\0 /		; divide
\0 ,		; ogonek
\0 '		; ring / degree
\0 "		; dieresis
\0 .		; dot above
^ 1		; to the first power
^ 3		; to the third power
^ 2		; to the second power
\0 o		;
\0 \d32		; space
END
