#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "lcd.h"

#define NUM_DIGITS 4

/*
 * font taken from TravelRec., with additions
 * Segments: https://upload.wikimedia.org/wikipedia/commons/b/b2/14-segment.png
 *
 *    aaaaaa aaaaaa
 *    f h   i   j b
 *    f  h  i  j  b
 *    f   h i j   b
 *     GGGGG ggggg
 *    e   m l k   c
 *    e  m  l  k  c
 *    e m   l   k c
 *    dddddd dddddd
 *
 */
#define FONT_ASCII_OFFSET '-'
static const uint16_t Font[] PROGMEM = {
//          mlkjihgGfedcba
        0b0000000011000000, // -
        0b0000000000000000, // . unused
        0b0000110000000000, // /
        0b0010010000111111, // 0
        0b0000010000000110, // 1
        0b0000000011011011, // 2
        0b0000000010001111, // 3
        0b0000000011100110, // 4
        0b0000000011101101, // 5
        0b0000000011111101, // 6
        0b0001010000000001, // 7
        0b0000000011111111, // 8
        0b0000000011101111, // 9
        0b0001001000000000, // :
        0b0010010000001110, // ;  used as Delta
        0b0000110000000000, // <
        0b0001001011000000, // =  used as '+'
        0b0010000100000000, // >
        0b0000000000000000, // ?  unused
        0b0000000011100011, // @  used as '°' character
        0b0000000011110111, // A
        0b0001001010001111, // B
        0b0000000000111001, // C
        0b0001001000001111, // D
        0b0000000001111001, // E
        0b0000000001110001, // F
        0b0000000010111101, // G
        0b0000000011110110, // H
        0b0001001000001001, // I
        0b0000000000011110, // J
        0b0000110001110000, // K
        0b0000000000111000, // L
        0b0000010100110110, // M
        0b0000100100110110, // N
        0b0000000000111111, // O
        0b0000000011110011, // P
        0b0000100000111111, // Q
        0b0000100011110011, // R
        0b0000000011101101, // S
        0b0001001000000001, // T
        0b0000000000111110, // U
        0b0010010000110000, // V
        0b0010100000110110, // W
        0b0010110100000000, // X
        0b0001010100000000, // Y
        0b0010010000001001, // Z
        //  mlkjihgGfedcba
        };

/*
 * ;		a	b	c	d	e	f	g1	g2	h	i	j	k	l	m
 * taken from TravelRec.
 */
#define SEGMENTS_PER_DIGIT 14
static const uint8_t Segments[] PROGMEM = { 126, 124, 44, 5, 7, 127, 47, 85, 87, 86, 125, 45, 46, 6, //left Digit
        123, 121, 1, 2, 4, 84, 43, 81, 83, 82, 122, 41, 42, 3, //middle left Digit
        137, 139, 59, 18, 16, 136, 56, 98, 96, 97, 138, 58, 57, 17, //middle right Digit
        140, 142, 22, 21, 19, 99, 60, 102, 100, 101, 141, 62, 61, 20 //right Digit
        };

static const uint8_t BargraphSegments[] PROGMEM = { 88, 48, 8, 9, 49, 89, 90, 50, 10, 11, 51, 91, 92, 52, 12, 13, 53, 93, 134, 94, 54, 14, 15, 55 };

/* Mo - So, LCDDR16 */
static const uint8_t WeekdaySegments[] PROGMEM = { 128, 129, 130, 131, 132, 133, 95 };

/* enum LCD_SYMBOLS */
static const uint8_t SymbolSegments[] PROGMEM = { 80, 120, 40, 23, 24, 64, 104, 144, 103, 143, 135, 0, 63 };

static void segmentOn(uint8_t segment)
{
    uint8_t volatile *segmentBase = &LCDDR0;
    segmentBase[segment / 8] |= (1 << segment % 8);
}

static void segmentOff(uint8_t segment)
{
    uint8_t volatile *segmentBase = &LCDDR0;
    segmentBase[segment / 8] &= ~(1 << segment % 8);
}

static void segmentSwitch(uint8_t segment, uint8_t on)
{
    if (on)
        segmentOn(segment);
    else
        segmentOff(segment);
}

static void displayDigit(uint16_t segments, uint8_t pos)
{
    uint8_t segmentOffset = pos * SEGMENTS_PER_DIGIT;
    uint8_t i;
    uint8_t segment;
    for (i = 0; i < SEGMENTS_PER_DIGIT; ++i) {
        segment = pgm_read_byte(&Segments[segmentOffset + i]);
        segmentSwitch(segment, (segments & (1 << i)) > 0);
    }
}

/**
 * Displays the bargraph.
 * @param bargraphOn each bit represents one hour on the bargraph.
 */
void displayBargraph(uint32_t bargraphOn)
{
    uint8_t i;
    for (i = 0; i < sizeof(BargraphSegments); ++i) {
        segmentSwitch(pgm_read_byte(&BargraphSegments[i]), (bargraphOn & (1 << i)) > 0);
    }
}

/**
 * Displays the weekday bar.
 * @param dayOn each bit represents one day to display
 */
void displayWeekday(uint8_t dayOn)
{
    uint8_t i;
    for (i = 0; i < 7; ++i) {
        segmentSwitch(pgm_read_byte(&WeekdaySegments[i]), dayOn & (1 << i));
    }
}

/**
 * Displays a symbol on the LCD. Only masked ones are taken into account.
 * @param on turn on these symbols
 * @param mask only touch symbols listed here
 */
void displaySymbols(LCD_SYMBOLS on, LCD_SYMBOLS mask)
{
    uint8_t i;
    for (i = 0; i < sizeof(SymbolSegments); ++i) {
        if (mask & (1 << i))
            segmentSwitch(pgm_read_byte(&SymbolSegments[i]), (on & (1 << i)) > 0);
    }

}

/**
 * Outputs one digit at given position.
 * @param c ASCII character to display
 * @param pos position on display, 0-NUM_DIGITS
 */
void displayAsciiDigit(char c, uint8_t pos)
{
    if (pos >= NUM_DIGITS)
        return;
    if (c == ' ') {
        displayDigit(0, pos);
        return;
    }
    uint8_t fontIndex = c - FONT_ASCII_OFFSET;
    uint16_t segments = pgm_read_word(&Font[fontIndex]);
    displayDigit(segments, pos);
}

/**
 * Outputs a string on the display, beginning on the left digit.
 * @param str string to display, it is displayed up to NUM_DIGITS
 */
void displayString(const char *str)
{
    uint8_t pos = 0;
    while (str[pos] && pos < NUM_DIGITS) {
        displayAsciiDigit(str[pos], pos);
        pos++;
    }
}

/// \brief Display a number consisting of up to four characters.
/// 
/// a minus sign is provided for negative numbers. (so we have
/// three remaining characters). Output will be left adjusted.
/// @param width leading with zeroes so we have given width
///
void displayNumber(int16_t num, int8_t width)
{
    char buf[NUM_DIGITS + 1];
    uint8_t i = NUM_DIGITS;
    char sign = '.';
    if (num < 0) {
        sign = '-';
        num = -num;
    }

    buf[NUM_DIGITS] = 0;
    while (i && num != 0) {
        buf[--i] = '0' + (num % 10);
        num /= 10;
    }
    width -= NUM_DIGITS - i; /* subtract already written chars */
    while (i && width > 0) {
        buf[--i] = sign; /* leading sign chars */
        sign = ' ';
        --width;
    }
    displayString(&buf[i]); /* begin output at first wanted character */
}

void lcdInit(void)
{
    LCDCRB = (1 << LCDCS) | (1 << LCDMUX1) | (1 << LCDMUX0) | (1 << LCDPM2) | (1 << LCDPM1) | (1 << LCDPM0);
    /*
     (1<<LCDCS)                            // Das LCD wird im asynchronen Modus (LCDCS-Bit=1)
     mit der Frequenz des Quarzes TOSC1 = 32.768Hz als LCD Clock betrieben.
     |(0<<LCD2B)                           // 1/3 bias is used
     |(1<<LCDMUX1)|(1<<LCDMUX0)            // 1/4 Duty; COM0:3;
     |(1<<LCDPM2)|(1<<LCDPM1)|(1<<LCDPM0); // SEG0:24
     */

    LCDFRR = (1 << LCDCD0);
    /*
     (0<<LCDPS2)|(0<<LCDPS1)|(0<<LCDPS0)    // N = 16
     |(0<<LCDCD2)|(0<<LCDCD1)|(1<<LCDCD0);  // D = 2
     K = 8 for duty = 1/4, 1/2, static
     f_frame = f_clk / (K * N * D)
     = 32kHz / (8 * 16 * 2) = 128 Hz

     */

    //TODO: Find optimum settings for low power
    LCDCCR = (1 << LCDDC2) | (0 << LCDDC1) | (0 << LCDDC0) | (/* TODO config.lcd_contrast*/10 << LCDCC0);
    /*
     (1<<LCDDC2)|(0<<LCDDC1)|(0<<LCDDC0)   // 575 µs
     // 3,1V
     |(config.lcd_contrast << LCDCC0);     // Set the initial LCD contrast level
     */

    LCDCRA = (1 << LCDEN) | (1 << LCDAB) | (0 << LCDIE) | (0 << LCDBL);
    /*
     (1<<LCDEN)    // Enable LCD
     |(1<<LCDAB)   // Low Power Waveform
     |(0<<LCDIE)   // disable Interrupt
     |(0<<LCDBL);  // No Blanking
     */
}

void lcdOff(void)
{
    LCDCRA &= ~(1 << LCDIE);
    // Note: This code is almost directly from the datasheet
    // Disable LCD
    // Wait until a new frame is started.
    while (!(LCDCRA & (1 << LCDIF)))
        ;
    // Set LCD Blanking and clear interrupt flag by writing a logical one to the flag.
    LCDCRA = (1 << LCDEN) | (1 << LCDIF) | (1 << LCDBL);
    // Wait until LCD Blanking is effective.
    while (!(LCDCRA & (1 << LCDIF)))
        ;
    // Disable LCD
    LCDCRA = 0;
}
