#ifndef __M_CTYPE_H
#define __M_CTYPE_H

#define m_isspace(x) ((x) == ' ' || (x) == '\t' || (x) == '\n' || (x) == '\r')
#define m_isdigit(x) ((x) >= '0' && (x) <= '9')
#define m_isxext(x) (((x) >= 'a' && (x) <= 'f') || ((x) >= 'A' && (x) <= 'F'))
#define m_isxdigit(x) (m_isdigit(x) || m_isxext(x))
#define m_isxal(x) (((x) >= 'G' && (x) <= 'Z') || ((x) >= 'g' && (x) <= 'z'))
#define m_isalpha(x) (m_isxal(x) || m_isxext(x))
#define m_isalnum(x) (m_isalpha(x) || m_isdigit(x))
#endif
