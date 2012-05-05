#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "drawing.h"
#include "BIOS.h"

#include "stm32f10x.h"

size_t _fread(void *ptr, size_t size, FILE *stream)
{
    return 0;
}

size_t _fwrite(const void *ptr, size_t size, FILE *stream)
{
    static int y = 220;
    static int x = 10;
    
    char buffer[2] = {0, 0};
    
    // Everything to USART1, stderr also to screen
    size_t i;
    char *p = (char*)ptr;
    for (i = 0; i < size; i++, p++)
    {
        if (stream == stderr)
        {
            buffer[0] = *p;
            
            if (*p != '\n')
                __Display_Str(x, y, 0xFFFF, 0, (u8*)buffer);
            
            x += FONT_WIDTH;
            if (x >= 390 || *p == '\n')
            {
                x = 10;
                y -= FONT_HEIGHT;
            }
        }
        
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = *p;
    }
    
    return i;
}