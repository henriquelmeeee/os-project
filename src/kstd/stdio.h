#pragma once

#ifndef _STDIO
#define _STDIO

#include "../Utils/Base.h"

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
