/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#include <console.h>

// Setting certain foreground color and background color combinations
// will cause the text to start blinking.
// This is due to FreeVGA doing that and it is needed to disable the
// 8th bit.
//
// Will skip it for now...
// TODO: Disable the blinking?
void SetConsoleColor(unsigned char foreColor, unsigned char backColor)
{
    consoleColor = (backColor << 4) | (foreColor & 0x0F);
    consoleColor |= 0 << 7;
}

void ClearScreen()
{
    for (y = 0; y < 25; y++)
    {
        for (x = 0; x < 80; x++)
        {
            myxp::uint16_t *tmp = videoMemory;
            tmp = videoMemory + (80 * y + x);
            *tmp = ' ' | (consoleColor << 8);
        }
    }

    x = 0;
    y = 0;
}

void ShiftConsoleColors(myxp::int8_t x_pos, myxp::int8_t y_pos)
{
    myxp::uint16_t *videoMemory = (myxp::uint16_t*)0xB8000;
    videoMemory[80*y_pos+x_pos] = (videoMemory[80*y_pos+x_pos] & 0x0F00) << 4 |
        (videoMemory[80*y_pos+x_pos] & 0xF000) >> 4 |
        (videoMemory[80*y_pos+x_pos] & 0x00FF);
}

void panic(char *str)
{
    asm volatile ("cli");

    SetConsoleColor(COLOR_GRAY, COLOR_BLACK);
    printf("[");
    SetConsoleColor(COLOR_RED, COLOR_BLACK);
    printf("PANIC");
    SetConsoleColor(COLOR_GRAY, COLOR_BLACK);
    printf("] ");
    printf(str);
    printf("\n");

    for(;;);
}

void panic_assert(const char *file, myxp::uint32_t line, const char *desc)
{
    asm volatile("cli");

    SetConsoleColor(COLOR_GRAY, COLOR_BLACK);
    printf("[");
    SetConsoleColor(COLOR_RED, COLOR_BLACK);
    printf("ASSERT FAIL");
    SetConsoleColor(COLOR_GRAY, COLOR_BLACK);
    printf("] ");
    printf(desc);
    printf("\nFile: ");
    printf(file);
    printf(":");
    printf(line);
    printf("\n");

    for(;;);
}

void errMsg(char *str)
{
    SetConsoleColor(COLOR_GRAY, COLOR_BLACK);
    printf("[");
    SetConsoleColor(COLOR_RED, COLOR_BLACK);
    printf("ERROR");
    SetConsoleColor(COLOR_GRAY, COLOR_BLACK);
    printf("] ");
    printf(str);
    printf("\n");
}

void sysmsg(char *str)
{
    SetConsoleColor(COLOR_GRAY, COLOR_BLACK);
    printf("[");
    SetConsoleColor(COLOR_YELLOW, COLOR_BLACK);
    printf("SYSTEM");
    SetConsoleColor(COLOR_GRAY, COLOR_BLACK);
    printf("] ");
    printf(str);
    printf("\n");
}

void printf(myxp::int32_t val)
{
    printf(myxp::string::itoa(val, 0, 10));
}

void printf(const char *str)
{
    printf((char*)str);
}

void printf(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        switch (str[i])
        {
        case '\n':
            x = 0;
            y++;
            break;

        default:
            // Print each character
            myxp::uint16_t *tmp = videoMemory;
            tmp = videoMemory + (80 * y + x);
            *tmp = str[i] | (consoleColor << 8);
            x++;
            break;
        }

        if (x >= 80)
        {
            x = 0;
            y++;
        }

        if (y >= 25)
        {
            ClearScreen();
        }
    }
}

void printfHex(myxp::uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}

void printfHex16(myxp::uint16_t key)
{
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}

void printfHex32(myxp::uint32_t key)
{
    printfHex((key >> 24) & 0xFF);
    printfHex((key >> 16) & 0xFF);
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}
