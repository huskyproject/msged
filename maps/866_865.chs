;
; This file is a charset conversion module in text form.
;
; This module converts CP866 to CP865 (translit conversion).
;
; Format: ID, version, level,
;         from charset, to charset,
;         128 entries: first & second byte
;         "END"
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
CP866     ; from set
CP865     ; to set
;
\0 \x41   ; 0x80   cyrillic capital letter a
\0 \x42   ; 0x81   cyrillic capital letter be
\0 \x56   ; 0x82   cyrillic capital letter ve
\0 \x47   ; 0x83   cyrillic capital letter ghe
\0 \x44   ; 0x84   cyrillic capital letter de
\0 \x45   ; 0x85   cyrillic capital letter ie
\x5a \x68 ; 0x86   cyrillic capital letter zhe
\0 \x5a   ; 0x87   cyrillic capital letter ze
\0 \x49   ; 0x88   cyrillic capital letter i
\0 \x4a   ; 0x89   cyrillic capital letter short i
\0 \x4b   ; 0x8a   cyrillic capital letter ka
\0 \x4c   ; 0x8b   cyrillic capital letter el
\0 \x4d   ; 0x8c   cyrillic capital letter em
\0 \x4e   ; 0x8d   cyrillic capital letter en
\0 \x4f   ; 0x8e   cyrillic capital letter o
\0 \x50   ; 0x8f   cyrillic capital letter pe
\0 \x52   ; 0x90   cyrillic capital letter er
\0 \x53   ; 0x91   cyrillic capital letter es
\0 \x54   ; 0x92   cyrillic capital letter te
\0 \x55   ; 0x93   cyrillic capital letter u
\0 \x46   ; 0x94   cyrillic capital letter ef
\0 \x48   ; 0x95   cyrillic capital letter ha
\x54 \x73 ; 0x96   cyrillic capital letter tse
\x43 \x68 ; 0x97   cyrillic capital letter che
\x53 \x68 ; 0x98   cyrillic capital letter sha
\x53 \x7a ; 0x99   cyrillic capital letter shcha
\0 \x60   ; 0x9a   cyrillic capital letter hard sign
\0 \x59   ; 0x9b   cyrillic capital letter yeru
\0 \x27   ; 0x9c   cyrillic capital letter soft sign
\0 \x45   ; 0x9d   cyrillic capital letter e
\x59 \x75 ; 0x9e   cyrillic capital letter yu
\x59 \x61 ; 0x9f   cyrillic capital letter ya
\0 \x61   ; 0xa0   cyrillic small letter a
\0 \x62   ; 0xa1   cyrillic small letter be
\0 \x76   ; 0xa2   cyrillic small letter ve
\0 \x67   ; 0xa3   cyrillic small letter ghe
\0 \x64   ; 0xa4   cyrillic small letter de
\0 \x65   ; 0xa5   cyrillic small letter ie
\x7a \x68 ; 0xa6   cyrillic small letter zhe
\0 \x7a   ; 0xa7   cyrillic small letter ze
\0 \x69   ; 0xa8   cyrillic small letter i
\0 \x6a   ; 0xa9   cyrillic small letter short i
\0 \x6b   ; 0xaa   cyrillic small letter ka
\0 \x6c   ; 0xab   cyrillic small letter el
\0 \x6d   ; 0xac   cyrillic small letter em
\0 \x6e   ; 0xad   cyrillic small letter en
\0 \x6f   ; 0xae   cyrillic small letter o
\0 \x70   ; 0xaf   cyrillic small letter pe
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
\0 \xdc   ; 0xdc   lower half block
\0 \xdd   ; 0xdd   left half block
\0 \xde   ; 0xde   right half block
\0 \xdf   ; 0xdf   upper half block
\0 \x72   ; 0xe0   cyrillic small letter er
\0 \x73   ; 0xe1   cyrillic small letter es
\0 \x74   ; 0xe2   cyrillic small letter te
\0 \x75   ; 0xe3   cyrillic small letter u
\0 \x66   ; 0xe4   cyrillic small letter ef
\0 \x68   ; 0xe5   cyrillic small letter ha
\x74 \x73 ; 0xe6   cyrillic small letter tse
\x63 \x68 ; 0xe7   cyrillic small letter che
\x73 \x68 ; 0xe8   cyrillic small letter sha
\x73 \x7a ; 0xe9   cyrillic small letter shcha
\0 \x60   ; 0xea   cyrillic small letter hard sign
\0 \x79   ; 0xeb   cyrillic small letter yeru
\0 \x27   ; 0xec   cyrillic small letter soft sign
\0 \x65   ; 0xed   cyrillic small letter e
\x79 \x75 ; 0xee   cyrillic small letter yu
\x79 \x61 ; 0xef   cyrillic small letter ya
\x59 \x6f ; 0xf0   cyrillic capital letter io
\x79 \x6f ; 0xf1   cyrillic small letter io
\x59 \x65 ; 0xf2   cyrillic capital letter ukrainian ie
\x79 \x65 ; 0xf3   cyrillic small letter ukrainian ie
\x59 \x69 ; 0xf4   cyrillic capital letter yi
\x79 \x69 ; 0xf5   cyrillic small letter yi
\x7e \x55 ; 0xf6   cyrillic capital letter short u
\x7e \x75 ; 0xf7   cyrillic small letter short u
\0 \xf8   ; 0xf8   degree sign
\0 \xf9   ; 0xf9   bullet operator
\0 \xfa   ; 0xfa   middle dot
\0 \xfb   ; 0xfb   sqare root
\0 \xfc   ; 0xfc   numero sign
\0 \xaf   ; 0xfd   currency sign
\0 \xfe   ; 0xfe   black square
\0 \xff   ; 0xff   no-break space
END
