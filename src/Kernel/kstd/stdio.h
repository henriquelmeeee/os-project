#pragma once

#ifndef _STDIO
#define _STDIO

#include "../Utils/Base.h"
#include <stdarg.h>

#define outb(port, val) \
  __asm__ volatile("outb %0, %1" : : "a"(static_cast<unsigned char>(val)), "dN"(static_cast<unsigned short>(port)))

inline char* itoa(int value, char* buffer, int base) {
    char* ptr = buffer;
    char* ptr1 = buffer;
    char tmp_char;
    int tmp_value;

    if (base < 2 || base > 36) {
        *buffer = '\0';
        return buffer;
    }

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return buffer;
    }

    if (value < 0 && base == 10) {
        value = -value;
        *ptr++ = '-';
    }

    while (value != 0) {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];
    }

    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return buffer;
}

#if 0
inline void dbg(const char *format, ...) __attribute__((format(printf, 1, 2)));

inline void dbg(const char *format, ...) {
    char *arg = (char *)&format;
    char buffer[256];  // buffer para a string de saída
    int buffer_index = 0;

    while (*format != '\0') {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {  // Inteiro
                    arg += 4;
                    int num = *(int *)arg;
                    char str[10];  // Temporário, assumindo que o inteiro não tem mais que 10 dígitos
                    // Suponha que itoa() é uma função que converte um int para string
                    itoa(num, str, 10);
                    for (int j = 0; str[j] != '\0'; ++j) {
                        buffer[buffer_index++] = str[j];
                    }
                    break;
                }
                case 's': {  // String
                    arg += 8;
                    char *str = *(char **)arg;
                    while (*str) {
                        buffer[buffer_index++] = *str++;
                    }
                    break;
                }
                // Casos adicionais aqui
                default:
                    buffer[buffer_index++] = *format;
                    break;
            }
        } else {
            buffer[buffer_index++] = *format;
        }
        format++;
    }
    buffer[buffer_index] = '\0';  // Terminar a string
    
    for (int i = 0; buffer[i] != '\0'; ++i) {
        outb(0x3F8, buffer[i]);  // Supondo que outb seja a função para escrever um byte em uma porta
    }
};
#endif

// TODO itoa() colocar aqui também
#if 0
inline void itoa(int value, char *str, int base) {

}
#endif

inline void reverse(char *str, int length) {
  int start = 0;
  int end = length - 1;
  while (start < end) {
    char temp = str[start];
    str[start] = str[end];
    str[end] = temp;
    start++;
    end--;
  }
}

inline void utoa(unsigned int value, char *str, int base) {
    int i = 0;

    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    while (value != 0) {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value = value / base;
    }

    str[i] = '\0';

    reverse(str, i);
}

inline void ltoa(long value, char *str, int base) {
    int i = 0;
    bool isNegative = false;

    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (value < 0 && base == 10) {
        isNegative = true;
        value = -value;
    }

    while (value != 0) {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value = value / base;
    }

    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    reverse(str, i);
}

// Unsigned Long to ASCII
inline void ultoa(unsigned long value, char *str, int base) {
    int i = 0;

    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    while (value != 0) {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value = value / base;
    }

    str[i] = '\0';

    reverse(str, i);
}

inline void dbg(const char *format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[256];
    int buffer_index = 0;

    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int num = va_arg(args, int);
                    char str[11];
                    itoa(num, str, 10);
                    for (char *ptr = str; *ptr; ++ptr) {
                        buffer[buffer_index++] = *ptr;
                    }
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char *);
                    while (*str) {
                        buffer[buffer_index++] = *str++;
                    }
                    break;
                }
                case 'l': {
                    format++;
                    if (*format == 'u') {
                        unsigned long num = va_arg(args, unsigned long);
                        char str[21];
                        ultoa(num, str, 10);
                        for (char *ptr = str; *ptr; ++ptr) {
                            buffer[buffer_index++] = *ptr;
                        }
                    } else {
                        long num = va_arg(args, long);
                        char str[21];
                        ltoa(num, str, 10);
                        for (char *ptr = str; *ptr; ++ptr) {
                            buffer[buffer_index++] = *ptr;
                        }
                    }
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    char str[11];
                    utoa(num, str, 10);
                    for (char *ptr = str; *ptr; ++ptr) {
                        buffer[buffer_index++] = *ptr;
                    }
                    break;
                }
                case 'c': {
                    char c = (char) va_arg(args, int);  // Char é promovido para int quando passado como argumento
                    buffer[buffer_index++] = c;
                    break;
                }
                case 'p': {
                    unsigned long p = (unsigned long) va_arg(args, void *);
                    char str[21];
                    ultoa(p, str, 16);  // Hexadecimal
                    for (char *ptr = str; *ptr; ++ptr) {
                        buffer[buffer_index++] = *ptr;
                    }
                    break;
                }
                default:
                    buffer[buffer_index++] = *format;
                    break;
            }
        } else {
            buffer[buffer_index++] = *format;
        }
        format++;
    }
    buffer[buffer_index] = '\0';

    for (int i = 0; buffer[i] != '\0'; ++i) {
        outb(0x3F8, buffer[i]);
    }
    outb(0x3F8, '\n');
    va_end(args);
}

#endif
