;
; This file is a charset conversion module in text form.
;
; This module converts ISO_8859-1 extended characters to IBM-PC characters.
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
LATIN-1         ; from set
CP437           ; to set
;
\0 \d128        ; (missing) These codes are unused in the LATIN-1 set.
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
\0 \xff         ; non-breaking space
\0 \xad         ; exclam downwards
\0 \x9b         ; cent
\0 \x9c         ; pound sterling
\0 \x0f         ; currency
\0 \x9d         ; Yen
\0 |            ; broken bar
\0 \x15         ; section
\x1 ?           ; dieresis
\0 c            ; copyright
\0 \xa6         ; ord feminine
\0 \xae         ; guillemot left
\0 \xaa         ; logical not
\0 -            ; soft hyphen (or em dash)
\0 R            ; registered trademark
\x1 ?           ; overbar (macron)
\0 \xf8         ; ring / degree
\0 \xf1         ; plusminus
\0 \xfd         ; superscript two (squared)
^ 3             ; superscript three (cubed)
\0 '            ; acute
\0 \xe6         ; mu
\0 \x14         ; paragraph
\0 \xfe         ; bullet
\0 ,            ; cedilla
^ 1             ; superscript one
\0 \xa7         ; ord masculine
\0 \xaf         ; guillemot right
\0 \xac         ; one quarter
\0 \xab         ; half
\x1 ?           ; three quarters
\0 \xa8         ; question downwards
\0 A            ; A grave
\0 A            ; A acute
\0 A            ; A circumflex
\0 A            ; A tilde
\0 \x8e         ; A dieresis
\0 \x8f         ; A ring
\0 \x92         ; AE
\0 \x80         ; C cedilla
\0 E            ; E grave
\0 \x90         ; E acute
\0 E            ; E circumflex
E e             ; E dieresis
\0 I            ; I grave
\0 I            ; I acute
\0 I            ; I circumflex
I e             ; I dieresis
\0 D            ; Eth
\0 \xa5         ; N tilde
\0 O            ; O grave
\0 O            ; O acute
\0 O            ; O circumflex
\0 O            ; O tilde
\0 \x99         ; O dieresis
\0 x            ; multiplication
;\0 \xed        ; O slash (mapping for CP437)
\0 \x9d         ; O slash (mapping for CP850/865)
\0 U            ; U grave
\0 U            ; U acute
\0 U            ; U circumflex
\0 \x9a         ; U dieresis
\0 Y            ; Y acute
\x1 ?           ; Thorn
\0 \xe1         ; german double s / beta
\0 \x85         ; a grave
\0 \xa0         ; a acute
\0 \x83         ; a circumflex
\0 a            ; a tilde
\0 \x84         ; a dieresis
\0 \x86         ; a ring
\0 \x91         ; ae
\0 \x87         ; c cedilla
\0 \x8a         ; e grave
\0 \x82         ; e acute
\0 \x88         ; e circumflex
\0 \x89         ; e dieresis
\0 \x8d         ; i grave
\0 \xa1         ; i acute
\0 \x8c         ; i circumflex
\0 \x8b         ; i dieresis
\0 \xe7         ; eth
\0 \xa4         ; n tilde
\0 \x95         ; o grave
\0 \xa2         ; o acute
\0 \x93         ; o circumflex
\0 o            ; o tilde
\0 \x94         ; o dieresis
\0 \xf6         ; division
;\0 o           ; o slash (mapping for CP 437, which does not have it)
\0 \x9b         ; o slash (mapping for CP 850/865)
\0 \x97         ; u grave
\0 \xa3         ; u acute
\0 \x96         ; u circumflex
\0 \x81         ; u dieresis
y e             ; y acute
\x1 ?           ; thorn
\0 \x98         ; y dieresis
END
