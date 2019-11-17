#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/wdt.h>

#include "config.h"
#include "menu.h"
#include "main.h"
#include "keys.h"
#include "lcd.h"
#include "timer.h"
#include "ntc.h"
#include "motor.h"
#include "control.h"
#include "programming.h"

typedef enum
{
    MENU_SELECTOR, MENU_FIRST, MENU_MAIN, MENU_PID, MENU_PROGRAM, MENU_OTA_UPDATE, MENU_VENT, MENU_TEMPERATURE_ADJUST, MENU_LAST
} MENU;

const char MenuText[][5] PROGMEM = { "MAIN", ".PID", "PROG", "OTAU", "VENT", "TADJ" };
const char PIDText[][5] PROGMEM = { ".K-P", ".K-D", ".K-I", " INT", "IMAX", "ELST" };

/**
 * requests the user to enter a number.
 * blocks until timeout has elapsed or user presses OK key.
 * over/underflowing minimum/maximum value wraps around.
 * @param min minimum input number
 * @param max maximum input number
 * @param init initial value
 * @param step step size
 * @param displayLength output that many characters on the display
 */
static int16_t inputNumber(int16_t min, int16_t max, int16_t init, int16_t step, uint8_t displayLength)
{
    TIMEOUT_INIT;
    int16_t value = init;
    while (TIMEOUT_OKAY) {
        int8_t valueChange = get_key_increment();
        /* exit on OK */
        if (get_key_press((1 << KEY_OK)))
            break;

        if (valueChange != 0)
            TIMEOUT_RENEW;

        displayNumber(value, displayLength);
        value += valueChange * step;
        if (value > max)
            value -= max - min;
        else if (value < min)
            value += max - min;
    }

    return value;
}

/**
 * UI for selecting a text message.
 * @param text: pointer to pointer to chars (array of strings) in program space
 * @param min/max selection bounds
 * @param init: start value
 * @param offset: array-Index = value - offset (signed)
 */
static uint8_t inputSelector(const char text[][5], uint8_t min, uint8_t max, uint8_t init, int8_t offset)
{
    TIMEOUT_INIT;
    int16_t value = init;

    while (TIMEOUT_OKAY) {
        char buf[5];

        // input
        int8_t valueChange = get_key_increment();

        if (get_key_press((1 << KEY_OK))) {
            break; // exit on OK
        }

        if (valueChange != 0) {
            TIMEOUT_RENEW;
        }

        // display
        buf[4] = 0;
        buf[0] = pgm_read_byte(&(text[value - offset][0]));
        buf[1] = pgm_read_byte(&(text[value - offset][1]));
        buf[2] = pgm_read_byte(&(text[value - offset][2]));
        buf[3] = pgm_read_byte(&(text[value - offset][3]));
        displayString(buf);

        // wrap-around
        value += valueChange;
        if (value > max)
            value -= max - min + 1;
        else if (value < min)
            value += max - min + 1;
    }

    return value;
}

void menu(void)
{
    static MENU currentMenu = MENU_MAIN;
    static uint8_t menuData = 0; /* number of screen in menu */
    int8_t valueChange;

    // enter menu
    if (get_key_long(1 << KEY_MENU)) {
        displaySymbols(0, LCD_BAG | LCD_DP | LCD_HOURS | LCD_INHOUSE | LCD_LOCK | LCD_MOON | LCD_OUTHOUSE | LCD_STAR);
        menuData = currentMenu;
        currentMenu = MENU_SELECTOR;
    }

    switch (currentMenu)
    {
    case MENU_SELECTOR:
    {
        uint8_t nextMenu = menuData;

        nextMenu = inputSelector(MenuText, MENU_FIRST + 1, MENU_LAST - 1, nextMenu, MENU_FIRST + 1);
        menuData = 0;
        currentMenu = nextMenu;
        break;
    }

        // this is the default state, not selected in the menu
    case MENU_MAIN:
    {
        // show current temperature unless setpoint is being adjusted
        if (menuData == 0) {
            displayNumber(getNtcTemperature(), 4);
        } else if (menuData == 1) {
            displayNumber(getNominalTemperature(), 4);
        }
        displaySymbols(LCD_DP, LCD_DP);

        // adjust setpoint
        valueChange = get_key_increment();
        if (valueChange != 0) {
            setNominalTemperature( getNominalTemperature() + valueChange * MANUAL_TEMPERATURE_STEP);
            menuData = 1; /* display this */
        }

        // disable program for X hours
        if (get_key_press(1 << KEY_CLOCK)) {
            int16_t hours;
            displayAsciiDigit('H', 3); /* suffix: hour */
            hours = inputNumber(0, 995, 5, 5, 3);
            dismissProgramChanges(hours * 6);
        }

        // return ??
        if (get_key_press(1 << KEY_OK)) {
            menuData = 1 - menuData;
        }

        break;
    }

    case MENU_PID:
    {
        int16_t value;
        if (get_key_short(1 << KEY_MENU)) {
            menuData = inputSelector(PIDText, 0, sizeof(PIDText) / sizeof(PIDText[0]), menuData, 0);
        }

        switch (menuData)
        {
        case 0:
            value = controller.k_p;
        break;
        case 1:
            value = controller.k_d;
        break;
        case 2:
            value = controller.k_i;
        break;
        case 3:
            value = controller.i_val / 100;
        break;
        case 4:
            value = controller.i_max / 10;
        break;
        case 5:
            value = controller.e_last;
        break;
        default:
            value = -999;
        }
        displayNumber(value, 4);

        if (get_key_increment()) {
            value = inputNumber(-999, 9999, value, 1, 4);
            switch (menuData)
            {
            case 0:
                controller.k_p = value;
            break;
            case 1:
                controller.k_d = value;
            break;
            case 2:
                controller.k_i = value;
            break;
            case 3:
                controller.i_val = value * 100;
            break;
            case 4:
                controller.i_max = value * 10;
            break;
            case 5:
                controller.e_last = value;
            break;
            }
        }
        break;
    }

    case MENU_VENT:
    {
        int16_t value = 0;
#if 0
        if (get_key_increment()) {
            menuData = inputNumber(0, 2, menuData, 1, 4);
        }
        switch (menuData)
        {
        case 0:
            value = MotorPosition;
        break;
        case 1:
            value = PositionValveOpen;
        break;
        case 2:
            value = PositionValveClosed;
        break;
        default:
            value = -999;
        }
#endif
        displayNumber(value, 4);
        break;
    }

    case MENU_TEMPERATURE_ADJUST:
    {
        displaySymbols(LCD_DP, LCD_DP);
        NTCOffset = inputNumber(-1000, 1000, NTCOffset, 10, 4);
        break;
    }

    case MENU_OTA_UPDATE:
    {
#ifdef BOOTLOADER
			BOOTLOADER_EXECUTE();
			#else
        // show message
#endif
        break;
    }

    default:
    {
        break;
    }
    }
}
