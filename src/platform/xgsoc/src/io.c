#include "io.h"

int chr_avail()
{
    return MEM_READ(UART_STATUS) & 1;
}

char get_chr()
{
    while (1) {
        if (MEM_READ(UART_STATUS) & 1) {
            unsigned int c = MEM_READ(UART_DATA);
            return (char)c;
        }
    }
}

void print_chr(char c)
{
    while(!(MEM_READ(UART_STATUS) & 2));
    MEM_WRITE(UART_DATA, c);
}

void print_buf(const char *s, size_t len)
{
    while (len > 0) {
        while(!(MEM_READ(UART_STATUS) & 2));
        MEM_WRITE(UART_DATA, *s);
        s++;
        len--;
    }
}

void print(const char *s)
{
    while (*s) {
        print_chr(*s);        
        s++;
    }
}
