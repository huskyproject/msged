;
; This file is a charset conversion module in text form.
;
; This module Converts IBM CP1125 to KOI8-RU
;
; Format: ID, version, level,
;         from charset, to charset,
;         128 entries: first & second byte
;     "END"
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
0         ; ID number
0         ; version number
;
2         ; level number
;
CP1125    ; from set
KOI8-R    ; to set
;
\0 \xE1   ; 0x80   cyrillic capital letter a
\0 \xE2   ; 0x81   cyrillic capital letter be
\0 \xF7   ; 0x82   cyrillic capital letter ve
\0 \xE7   ; 0x83   cyrillic capital letter ghe
\0 \xE4   ; 0x84   cyrillic capital letter de
\0 \xE5   ; 0x85   cyrillic capital letter ie
\0 \xF6   ; 0x86   cyrillic capital letter zhe
\0 \xFA   ; 0x87   cyrillic capital letter ze
\0 \xE9   ; 0x88   cyrillic capital letter i
\0 \xEA   ; 0x89   cyrillic capital letter short i
\0 \xEB   ; 0x8a   cyrillic capital letter ka
\0 \xEC   ; 0x8b   cyrillic capital letter el
\0 \xED   ; 0x8c   cyrillic capital letter em
\0 \xEE   ; 0x8d   cyrillic capital letter en
\0 \xEF   ; 0x8e   cyrillic capital letter o
\0 \xF0   ; 0x8f   cyrillic capital letter pe
\0 \xF2   ; 0x90   cyrillic capital letter er
\0 \xF3   ; 0x91   cyrillic capital letter es
\0 \xF4   ; 0x92   cyrillic capital letter te
\0 \xF5   ; 0x93   cyrillic capital letter u
\0 \xE6   ; 0x94   cyrillic capital letter ef
\0 \xE8   ; 0x95   cyrillic capital letter ha
\0 \xE3   ; 0x96   cyrillic capital letter tse
\0 \xFE   ; 0x97   cyrillic capital letter che
\0 \xFB   ; 0x98   cyrillic capital letter sha
\0 \xFD   ; 0x99   cyrillic capital letter shcha
\0 \xFF   ; 0x9a   cyrillic capital letter hard sign
\0 \xF9   ; 0x9b   cyrillic capital letter yeru
\0 \xF8   ; 0x9c   cyrillic capital letter soft sign
\0 \xFC   ; 0x9d   cyrillic capital letter e
\0 \xE0   ; 0x9e   cyrillic capital letter yu
\0 \xF1   ; 0x9f   cyrillic capital letter ya
\0 \xC1   ; 0xA0   cyrillic small letter a
\0 \xC2   ; 0xA1   cyrillic small letter be
\0 \xD7   ; 0xA2   cyrillic small letter ve
\0 \xC7   ; 0xA3   cyrillic small letter ghe
\0 \xC4   ; 0xA4   cyrillic small letter de
\0 \xC5   ; 0xA5   cyrillic small letter ie
\0 \xD6   ; 0xA6   cyrillic small letter zhe
\0 \xDA   ; 0xA7   cyrillic small letter ze
\0 \xC9   ; 0xA8   cyrillic small letter i
\0 \xCA   ; 0xA9   cyrillic small letter short i
\0 \xCB   ; 0xAa   cyrillic small letter ka
\0 \xCC   ; 0xAb   cyrillic small letter el
\0 \xCD   ; 0xAc   cyrillic small letter em
\0 \xCE   ; 0xAd   cyrillic small letter en
\0 \xCF   ; 0xAe   cyrillic small letter o
\0 \xD0   ; 0xAf   cyrillic small letter pe
\0 #      ; 0xb0   light shade
\0 #      ; 0xb1   medium shade
\0 #      ; 0xb2   dark shade
\0 |      ; 0xb3   box drawings light vertical
\0 |      ; 0xb4   box drawings light vertical and left
\0 |      ; 0xb5   box drawings vertical single and left double
\0 |      ; 0xb6   box drawings vertical double and left single
\0 +      ; 0xb7   box drawings down double and left single
\0 +      ; 0xb8   box drawings down single and left double
\0 |      ; 0xb9   box drawings double vertical and left
\0 |      ; 0xba   box drawings double vertical
\0 +      ; 0xbb   box drawings double down and left
\0 +      ; 0xbc   box drawings double up and left
\0 +      ; 0xbd   box drawings up double and left single
\0 +      ; 0xbe   box drawings up single and left double
\0 +      ; 0xbf   box drawings light down and left
\0 +      ; 0xc0   box drawings light up and right
\0 -      ; 0xc1   box drawings light up and horizontal
\0 -      ; 0xc2   box drawings light down and horizontal
\0 |      ; 0xc3   box drawings light vertical and right
\0 -      ; 0xc4   box drawings light horizontal
\0 +      ; 0xc5   box drawings light vertical and horizontal
\0 |      ; 0xc6   box drawings vertical single and right double
\0 |      ; 0xc7   box drawings vertical double and right single
\0 +      ; 0xc8   box drawings double up and right
\0 +      ; 0xc9   box drawings double down and right
\0 =      ; 0xca   box drawings double up and horizontal
\0 =      ; 0xcb   box drawings double down and horizontal
\0 |      ; 0xcc   box drawings double vertical and right
\0 =      ; 0xcd   box drawings double horizontal
\0 +      ; 0xce   box drawings double vertical and horizontal
\0 =      ; 0xcf   box drawings up single and horizontal double
\0 -      ; 0xd0   box drawings up double and horizontal single
\0 =      ; 0xd1   box drawings down single and horizontal double
\0 -      ; 0xd2   box drawings down double and horizontal single
\0 +      ; 0xd3   box drawings up double and right single
\0 +      ; 0xd4   box drawings up single and right double
\0 +      ; 0xd5   box drawings down single and right double
\0 +      ; 0xd6   box drawings down double and right single
\0 |      ; 0xd7   box drawings vertical double and horizontal single
\0 +      ; 0xd8   box drawings vertical single and horizontal double
\0 +      ; 0xd9   box drawings light up and left
\0 +      ; 0xda   box drawings light down and right
\0 #      ; 0xdb   full block
\0 n      ; 0xdc   lower half block
\0 |      ; 0xdd   left half block
\0 |      ; 0xde   right half block
\0 ~      ; 0xdf   upper half block
\0 \xD2   ; 0xE0   cyrillic small letter er
\0 \xD3   ; 0xE1   cyrillic small letter es
\0 \xD4   ; 0xE2   cyrillic small letter te
\0 \xD5   ; 0xE3   cyrillic small letter u
\0 \xC6   ; 0xE4   cyrillic small letter ef
\0 \xC8   ; 0xE5   cyrillic small letter ha
\0 \xC3   ; 0xE6   cyrillic small letter tse
\0 \xDE   ; 0xE7   cyrillic small letter che
\0 \xDB   ; 0xE8   cyrillic small letter sha
\0 \xDD   ; 0xE9   cyrillic small letter shcha
\0 \xDF   ; 0xEa   cyrillic small letter hard sign
\0 \xD9   ; 0xEb   cyrillic small letter yeru
\0 \xD8   ; 0xEc   cyrillic small letter soft sign
\0 \xDC   ; 0xEd   cyrillic small letter e
\0 \xC0   ; 0xEe   cyrillic small letter yu
\0 \xD1   ; 0xEf   cyrillic small letter ya
\0 \xB3   ; 0xf0   cyrillic capital letter io
\0 \xA3   ; 0xf1   cyrillic small letter io
\0 \xBD   ; 0xf2   cyrillic capital letter ghe with upturn
\0 \xAD   ; 0xf3   cyrillic small letter ghe with upturn
\0 \xB4   ; 0xf4   cyrillic capital letter ukrainian ie 
\0 \xA4   ; 0xf5   cyrillic small letter ukrainian ie
\0 \xB6   ; 0xf6   cyrillic capital letter byelorussian-ukrainian i
\0 \xA6   ; 0xf7   cyrillic small letter byelorussian-ukrainian i
\0 \xB7   ; 0xf8   cyrillic capital letter yi
\0 \xA7   ; 0xf9   cyrillic small letter yi
\0 -      ; 0xfa   middle dot
\x1 ?     ; 0xfb   sqare root
^ n       ; 0xfc   numero sign
^ 2       ; 0xfd   currency sign
\0 o      ; 0xfe   black square
\0 \d32   ; 0xff   no-break space
END
