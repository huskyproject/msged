;
; This file is a charset conversion module in text form.
;
; This module Converts IBM extended characters to ASCII,
; or to UK characters.
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
2               ; level number
;
IBMPC		; from set
SWEDISH         ; to set (change to UK for UK character set)
;
\0 \d128        ; C with cedilla
\0 \d129        ; u dieresis
\0 \d130        ; e acute
\0 \d131        ; a circumflex
\0 {            ; a dieresis
\0 \d133        ; a grave
\0 }            ; a ring
\0 \d135        ; c cedilla
\0 \d136        ; e circumflex
\0 \d137        ; e dieresis
\0 \d138        ; e grave
\0 \d139        ; i dieresis
\0 \d140        ; i circumflex
\0 \d141        ; i grave
\0 [            ; A dieresis
\0 ]            ; A ring
\0 \d064        ; E acute
\0 \d145        ; ae
\0 \d146        ; AE
\0 \d147        ; o circumflex
\0 |            ; o dieresis
\0 \d149        ; o acute
\0 \d150        ; u circumflex
\0 \d151        ; u grave
\0 \d152        ; y dieresis
\0 \\           ; O dieresis
\0 \d154          ; U dieresis
\0 \d155          ; cent
\0 \d156          ; pound sterling
\0 \d157          ; yen
\0 \d158          ; Pt
\0 \d159          ; florin
\0 \d160          ; a acute
\0 \d161          ; i grave
\0 \d162          ; o grave
\0 \d163          ; u grave
\0 \d164          ; n tilde
\0 \d165          ; N tilde
\0 \d166          ; ord feminine
\0 \d167          ; ord masculine
\0 \d168          ; question downwards
\0 \d169          ;
\0 \d170          ; logical not
\0 \d171          ; half fraction
\0 \d172          ; quarter fraction
\0 \d173          ; exclam downwards
\0 \d174          ; guillemot left
\0 \d175          ; guillemot right
\0 \d176          ;
\0 \d177          ;
\0 \d178          ;
\0 \d179          ;
\0 \d180          ;
\0 \d181          ;
\0 \d182          ;
\0 \d183          ;
\0 \d184          ;
\0 \d185          ;
\0 \d186          ;
\0 \d187          ;
\0 \d188          ;
\0 \d189          ;
\0 \d190          ;
\0 \d191          ;
\0 \d192          ;
\0 \d193          ;
\0 \d194          ;
\0 \d195          ;
\0 \d196          ;
\0 \d197          ;
\0 \d198          ;
\0 \d199          ;
\0 \d200          ;
\0 \d201           ;
\0 \d202           ;
\0 \d203           ;
\0 \d204           ;
\0 \d205           ;
\0 \d206           ;
\0 \d207           ;
\0 \d208           ;
\0 \d209           ;
\0 \d210           ;
\0 \d211           ;
\0 \d212           ;
\0 \d213           ;
\0 \d214           ;
\0 \d215           ;
\0 \d216           ;
\0 \d217           ;
\0 \d218           ;
\0 \d219           ;
\0 \d220           ;
\0 \d221           ;
\0 \d222           ;
\0 \d223           ;
\0 \d224           ; alpha
\0 \d225           ; german double s (misused as Beta)
\0 \d226           ; Gamma
\0 \d227           ; pi
\0 \d228           ; Sigma (summation)
\0 \d229           ; sigma
\0 \d230           ; mu
\0 \d231           ; gamma
\0 \d232           ; Phi
\0 \d233           ; Theta
\0 \d234           ; Omega
\0 \d235           ; delta
\0 \d236           ; infinity
\0 \d237           ; o slash
\0 \d238           ; element
\0 \d239           ; intersection
\0 \d240           ; equivalence
\0 \d241           ; plusminus
\0 \d242           ; greater equals
\0 \d243           ; smaller equals
\0 \d244           ; integral top
\0 \d245           ; integral bottom
\0 \d246           ; divide
\0 \d247           ; approx.
\0 \d248           ; ring / degree
\0 \d249           ; centered dot
\0 \d250           ; en dash
\0 \d251           ; radical
\0 \d252           ; to the n'th power
\0 \d253           ; to the second power
\0 \d254           ;
\0 \d255           ; space
END
