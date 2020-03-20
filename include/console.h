/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__CONSOLE_H
#define __MYXP__CONSOLE_H

#include <lib/string.h>
#include <lib/types.h>

#define COLOR_BLACK     0x0
#define COLOR_BLUE      0x1
#define COLOR_GREEN     0x2
#define COLOR_CYAN      0x3
#define COLOR_RED       0x4
#define COLOR_PURPLE    0x5
#define COLOR_BROWN     0x6
#define COLOR_GRAY      0x7
#define COLOR_DARK_GRAY     0x8
#define COLOR_LIGHT_BLUE    0x9
#define COLOR_LIGHT_GREEN   0xA
#define COLOR_LIGHT_CYAN    0xB
#define COLOR_LIGHT_RED     0xC
#define COLOR_LIGHT_PURPLE  0xD
#define COLOR_YELLOW        0xE
#define COLOR_WHITE         0xF

static myxp::uint32_t consoleColor = 0x0F;
// The address for printing characters to the screen starts at 0xB8000
static myxp::uint16_t *videoMemory = (myxp::uint16_t*)0xB8000;

static myxp::uint8_t x = 0;
static myxp::uint8_t y = 0;

void ShiftConsoleColors(myxp::int8_t xpos, myxp::int8_t y_pos);
void SetConsoleColor(unsigned char foreColor, unsigned char backColor);
void ClearScreen();
void printf(const char *str);
void printf(char *str);
void printf(myxp::int32_t val);
void sysmsg(char *str);
void errMsg(char *str);
void panic(char *str);
void panic_assert(const char *file, myxp::uint32_t line, const char *desc);
void printfHex(myxp::uint8_t key);
void printfHex16(myxp::uint16_t key);
void printfHex32(myxp::uint32_t key);

#endif