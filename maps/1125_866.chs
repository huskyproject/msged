;
; This file is a charset conversion module in text form.
;
; This module Converts IBM CP1125 to IBM CP866.
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
CP866     ; to set
;
\0 \x80   ; 0x80   cyrillic capital letter a
\0 \x81   ; 0x81   cyrillic capital letter be
\0 \x82   ; 0x82   cyrillic capital letter ve
\0 \x83   ; 0x83   cyrillic capital letter ghe
\0 \x84   ; 0x84   cyrillic capital letter de
\0 \x85   ; 0x85   cyrillic capital letter ie
\0 \x86   ; 0x86   cyrillic capital letter zhe
\0 \x87   ; 0x87   cyrillic capital letter ze
\0 \x88   ; 0x88   cyrillic capital letter i
\0 \x89   ; 0x89   cyrillic capital letter short i
\0 \x8a   ; 0x8a   cyrillic capital letter ka
\0 \x8b   ; 0x8b   cyrillic capital letter el
\0 \x8c   ; 0x8c   cyrillic capital letter em
\0 \x8d   ; 0x8d   cyrillic capital letter en
\0 \x8e   ; 0x8e   cyrillic capital letter o
\0 \x8f   ; 0x8f   cyrillic capital letter pe
\0 \x90   ; 0x90   cyrillic capital letter er
\0 \x91   ; 0x91   cyrillic capital letter es
\0 \x92   ; 0x92   cyrillic capital letter te
\0 \x93   ; 0x93   cyrillic capital letter u
\0 \x94   ; 0x94   cyrillic capital letter ef
\0 \x95   ; 0x95   cyrillic capital letter ha
\0 \x96   ; 0x96   cyrillic capital letter tse
\0 \x97   ; 0x97   cyrillic capital letter che
\0 \x98   ; 0x98   cyrillic capital letter sha
\0 \x99   ; 0x99   cyrillic capital letter shcha
\0 \x9a   ; 0x9a   cyrillic capital letter hard sign
\0 \x9b   ; 0x9b   cyrillic capital letter yeru
\0 \x9c   ; 0x9c   cyrillic capital letter soft sign
\0 \x9d   ; 0x9d   cyrillic capital letter e
\0 \x9e   ; 0x9e   cyrillic capital letter yu
\0 \x9f   ; 0x9f   cyrillic capital letter ya
\0 \xa0   ; 0xa0   cyrillic small letter a
\0 \xa1   ; 0xa1   cyrillic small letter be
\0 \xa2   ; 0xa2   cyrillic small letter ve
\0 \xa3   ; 0xa3   cyrillic small letter ghe
\0 \xa4   ; 0xa4   cyrillic small letter de
\0 \xa5   ; 0xa5   cyrillic small letter ie
\0 \xa6   ; 0xa6   cyrillic small letter zhe
\0 \xa7   ; 0xa7   cyrillic small letter ze
\0 \xa8   ; 0xa8   cyrillic small letter i
\0 \xa9   ; 0xa9   cyrillic small letter short i
\0 \xaa   ; 0xaa   cyrillic small letter ka
\0 \xab   ; 0xab   cyrillic small letter el
\0 \xac   ; 0xac   cyrillic small letter em
\0 \xad   ; 0xad   cyrillic small letter en
\0 \xae   ; 0xae   cyrillic small letter o
\0 \xaf   ; 0xaf   cyrillic small letter pe
\0 \xb0   ; 0xb0   light shade
\0 \xb1   ; 0xb1   medium shade
\0 \xb2   ; 0xb2   dark shade
\0 \xb3   ; 0xb3   box drawings light vertical
\0 \xb4   ; 0xb4   box drawings light vertical and left
\0 \xb5   ; 0xb5   box drawings vertical single and left double
\0 \xb6   ; 0xb6   box drawings vertical double and left single
\0 \xb7   ; 0xb7   box drawings down double and left single
\0 \xb8   ; 0xb8   box drawings down single and left double
\0 \xb9   ; 0xb9   box drawings double vertical and left
\0 \xba   ; 0xba   box drawings double vertical
\0 \xbb   ; 0xbb   box drawings double down and left
\0 \xbc   ; 0xbc   box drawings double up and left
\0 \xbd   ; 0xbd   box drawings up double and left single
\0 \xbe   ; 0xbe   box drawings up single and left double
\0 \xbf   ; 0xbf   box drawings light down and left
\0 \xc0   ; 0xc0   box drawings light up and right
\0 \xc1   ; 0xc1   box drawings light up and horizontal
\0 \xc2   ; 0xc2   box drawings light down and horizontal
\0 \xc3   ; 0xc3   box drawings light vertical and right
\0 \xc4   ; 0xc4   box drawings light horizontal
\0 \xc5   ; 0xc5   box drawings light vertical and horizontal
\0 \xc6   ; 0xc6   box drawings vertical single and right double
\0 \xc7   ; 0xc7   box drawings vertical double and right single
\0 \xc8   ; 0xc8   box drawings double up and right
\0 \xc9   ; 0xc9   box drawings double down and right
\0 \xca   ; 0xca   box drawings double up and horizontal
\0 \xcb   ; 0xcb   box drawings double down and horizontal
\0 \xcc   ; 0xcc   box drawings double vertical and right
\0 \xcd   ; 0xcd   box drawings double horizontal
\0 \xce   ; 0xce   box drawings double vertical and horizontal
\0 \xcf   ; 0xcf   box drawings up single and horizontal double
\0 \xd0   ; 0xd0   box drawings up double and horizontal single
\0 \xd1   ; 0xd1   box drawings down single and horizontal double
\0 \xd2   ; 0xd2   box drawings down double and horizontal single
\0 \xd3   ; 0xd3   box drawings up double and right single
\0 \xd4   ; 0xd4   box drawings up single and right double
\0 \xd5   ; 0xd5   box drawings down single and right double
\0 \xd6   ; 0xd6   box drawings down double and right single
\0 \xd7   ; 0xd7   box drawings vertical double and horizontal single
\0 \xd8   ; 0xd8   box drawings vertical single and horizontal double
\0 \xd9   ; 0xd9   box drawings light up and left
\0 \xda   ; 0xda   box drawings light down and right
\0 \xdb   ; 0xdb   full block
\0 \xdc   ; 0xdc   loWer half block
\0 \xdd   ; 0xdd   left half block
\0 \xde   ; 0xde   right half block
\0 \xdf   ; 0xdf   upper half block
\0 \xe0   ; 0xe0   cyrillic small letter er
\0 \xe1   ; 0xe1   cyrillic small letter es
\0 \xe2   ; 0xe2   cyrillic small letter te
\0 \xe3   ; 0xe3   cyrillic small letter u
\0 \xe4   ; 0xe4   cyrillic small letter ef
\0 \xe5   ; 0xe5   cyrillic small letter ha
\0 \xe6   ; 0xe6   cyrillic small letter tse
\0 \xe7   ; 0xe7   cyrillic small letter che
\0 \xe8   ; 0xe8   cyrillic small letter sha
\0 \xe9   ; 0xe9   cyrillic small letter shcha
\0 \xea   ; 0xea   cyrillic small letter hard sign
\0 \xeb   ; 0xeb   cyrillic small letter yeru
\0 \xec   ; 0xec   cyrillic small letter soft sign
\0 \xed   ; 0xed   cyrillic small letter e
\0 \xee   ; 0xee   cyrillic small letter yu
\0 \xef   ; 0xef   cyrillic small letter ya
\0 \xf0   ; 0xf0   cyrillic capital letter io
\0 \xf1   ; 0xf1   cyrillic small letter io
\x83 \x27 ; 0xf2   cyrillic capital letter ghe with upturn
\xa3 \x27 ; 0xf3   cyrillic small letter ghe with upturn
\0 \xf2   ; 0xf4   cyrillic capital letter ukrainian ie
\0 \xf3   ; 0xf5   cyrillic small letter ukrainian ie
\0 \x49   ; 0xf6   cyrillic capital letter byelorussian-ukrainian i
\0 \x69   ; 0xf7   cyrillic small letter byelorussian-ukrainian i
\0 \xf4   ; 0xf8   cyrillic capital letter yi
\0 \xf5   ; 0xf9   cyrillic small letter yi
\0 \xfa   ; 0xfa   middle dot
\0 \xfb   ; 0xfb   square root
\0 \xfc   ; 0xfc   numero sign
\0 \xfd   ; 0xfd   currency sign
\0 \xfe   ; 0xfe   black square
\0 \xff   ; 0xff   no-break space
END
