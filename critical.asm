; This code released to the public domain by Matthew Parker

.model large, c

ifdef WATCOM
install24h EQU _install24h
remove24h  EQU _remove24h
endif

public install24h, remove24h

.code

install24h proc
    push    ds
    push    es
    mov     ax,3524h             ;Get address of int 24h
    int     21h
    mov     word ptr[old24h],bx  ;Store old address for later use
    mov     word ptr[old24h+2],es
    push    cs
    pop     ds
    mov     dx,offset int24h
    mov     ax,2524h
    int     21h
    pop     es
    pop     ds
    ret
install24h endp

remove24h proc
    push    ds
    lds     dx,[old24h]
    mov     ax,2524h
    int     21h
    pop     ds
    ret
remove24h endp

int24h proc
    mov    al,3  ;Fail disk operation
    iret
int24h endp

.data

old24h     dd ?

end
