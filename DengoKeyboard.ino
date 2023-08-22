/*******************************************************************************
   This file is part of PsxNewLib.
 *                                                                             *
   Copyright (C) 2019-2020 by SukkoPera <software@sukkology.net>
 *                                                                             *
   PsxNewLib is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
 *                                                                             *
   PsxNewLib is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 *                                                                             *
   You should have received a copy of the GNU General Public License
   along with PsxNewLib. If not, see http://www.gnu.org/licenses.
 *******************************************************************************

   This sketch will dump to serial whatever is done on a PSX controller. It is
   an excellent way to test that all buttons/sticks are read correctly.

   It's missing support for analog buttons, that will come in the future.

   This example drives the controller by bitbanging the protocol, there is
   another similar one using the hardware SPI support.
*/

#include <DigitalIO.h>
#include <PsxControllerBitBang.h>
#include "TM1651.h"
#include <Keyboard.h>

#include <avr/pgmspace.h>
typedef const __FlashStringHelper * FlashStr;
typedef const byte* PGM_BYTES_P;
#define PSTR_TO_F(s) reinterpret_cast<const __FlashStringHelper *> (s)

// These can be changed freely when using the bitbanged protocol
const byte PIN_PS2_ATT = 10;
const byte PIN_PS2_CMD = 11;
const byte PIN_PS2_DAT = 12;
const byte PIN_PS2_CLK = 13;

#define LED_CLK 3//pins definitions for TM1651 and can be changed to other ports       
#define LED_DIO 2
#define LED_COUNTTO 16 //digits from 0-f (use 10 to count from 0-9)

TM1651 Display(LED_CLK, LED_DIO);

#define KEYPRESS 100


const char buttonSelectName[] PROGMEM = "Select";
const char buttonL3Name[] PROGMEM = "L3";
const char buttonR3Name[] PROGMEM = "R3";
const char buttonStartName[] PROGMEM = "Start";
const char buttonUpName[] PROGMEM = "Up";
const char buttonRightName[] PROGMEM = "Right";
const char buttonDownName[] PROGMEM = "Down";
const char buttonLeftName[] PROGMEM = "Left";
const char buttonL2Name[] PROGMEM = "L2";
const char buttonR2Name[] PROGMEM = "R2";
const char buttonL1Name[] PROGMEM = "L1";
const char buttonR1Name[] PROGMEM = "R1";
const char buttonTriangleName[] PROGMEM = "Triangle";
const char buttonCircleName[] PROGMEM = "Circle";
const char buttonCrossName[] PROGMEM = "Cross";
const char buttonSquareName[] PROGMEM = "Square";

const char* const psxButtonNames[PSX_BUTTONS_NO] PROGMEM = {
  buttonSelectName,
  buttonL3Name,
  buttonR3Name,
  buttonStartName,
  buttonUpName,
  buttonRightName,
  buttonDownName,
  buttonLeftName,
  buttonL2Name,
  buttonR2Name,
  buttonL1Name,
  buttonR1Name,
  buttonTriangleName,
  buttonCircleName,
  buttonCrossName,
  buttonSquareName
};

byte psxButtonToIndex (PsxButtons psxButtons) {
  byte i;

  for (i = 0; i < PSX_BUTTONS_NO; ++i) {
    if (psxButtons & 0x01) {
      break;
    }

    psxButtons >>= 1U;
  }

  return i;
}

FlashStr getButtonName (PsxButtons psxButton) {
  FlashStr ret = F("");

  byte b = psxButtonToIndex (psxButton);
  if (b < PSX_BUTTONS_NO) {
    PGM_BYTES_P bName = reinterpret_cast<PGM_BYTES_P> (pgm_read_ptr (&(psxButtonNames[b])));
    ret = PSTR_TO_F (bName);
  }

  return ret;
}

void dumpButtons (PsxButtons psxButtons) {
  static PsxButtons lastB = 0;

  if (psxButtons != lastB) {
    lastB = psxButtons;     // Save it before we alter it

    Serial.print (F("Pressed: "));

    for (byte i = 0; i < PSX_BUTTONS_NO; ++i) {
      byte b = psxButtonToIndex (psxButtons);
      if (b < PSX_BUTTONS_NO) {
        PGM_BYTES_P bName = reinterpret_cast<PGM_BYTES_P> (pgm_read_ptr (&(psxButtonNames[b])));
        Serial.print (PSTR_TO_F (bName));
      }

      psxButtons &= ~(1 << b);

      if (psxButtons != 0) {
        Serial.print (F(", "));
      }
    }

    Serial.println ();
  }
}

void dengo(PsxButtons psxButtons) {
  static PsxButtons lastB = 0;
  static uint8_t lastPowerState = 0;
  static uint8_t lastBreakState = 0;
  uint8_t powerState, powerStep;
  uint8_t breakState, breakStep;

  if (psxButtons != lastB) {
    lastB = psxButtons;     // Save it before we alter it

    uint16_t powerHandle = ( psxButtons ) & 0x10f0;
    uint16_t breakHandle = ( psxButtons ) & 0x0f00;

    //    Serial.print (powerHandle, HEX);
    //    Serial.println ();

    switch (powerHandle) {
      case 0xF0:
        //      Serial.println(F("P0"));
        powerState = 0;
        break;
      case 0x1070:
        //     Serial.println(F("P1"));
        powerState = 1;
        break;
      case 0x70:
        //     Serial.println(F("P2"));
        powerState = 2;
        break;
      case 0x10D0:
        //   Serial.println(F("P3"));
        powerState = 3;
        break;
      case 0xD0:
        //    Serial.println(F("P4"));
        powerState = 4;
        break;
      case 0x1050:
        //    Serial.println(F("P5"));
        powerState = 5;
        break;
      //     case 0x50:
      //        Serial.println(F("N"));
      //      break;

      default:
        powerState = lastPowerState;
        break;
    }

    //  Serial.print (breakHandle, HEX);
    //  Serial.println ();

    switch (breakHandle) {
      case 0xB00:
        //     Serial.println(F("B0"));
        breakState = 0;
        break;
      case 0xE00:
        //   Serial.println(F("B1"));
        breakState = 1;
        break;
      case 0xA00:
        //       Serial.println(F("B2"));
        breakState = 2;
        break;
      case 0x700:
        //     Serial.println(F("B3"));
        breakState = 3;
        break;
      case 0x300:
        //   Serial.println(F("B4"));
        breakState = 4;
        break;
      case 0x600:
        //     Serial.println(F("B5"));
        breakState = 5;
        break;
      case 0x200:
        //   Serial.println(F("B6"));
        breakState = 6;
        break;
      case 0xD00:
        //   Serial.println(F("B7"));
        breakState = 7;
        break;
      case 0x900:
        //     Serial.println(F("B8"));
        breakState = 8;
        break;
      case 0x00:
        //       Serial.println(F("EB"));
        breakState = 9;
        break;

      default:
        breakState = lastBreakState;
        break;
    }

    if ((psxButtons & 0x8000) != 0) {
      Serial.println(F("A"));
    }
    if ((psxButtons & 0x4000) != 0) {
      Serial.println(F("B"));
    }
    if ((psxButtons & 0x2000) != 0) {
      Serial.println(F("C"));
    }
    if ((psxButtons & 0x1) != 0) {
      Serial.println(F("SEL"));
    }
    if ((psxButtons & 0x8) != 0) {
      Serial.println(F("START"));
    }


    // display handle status

    Display.displayNum(0, 16);
    if ( breakState == 9 ) {
      Display.displayNum(1, 14);
      Display.displayNum(2, 11);
    } else if ( breakState == 0 ) {
      Display.displayNum(1, 16);
      Display.displayNum(2, powerState);
    } else {
      Display.displayNum(1, 11);
      Display.displayNum(2, breakState);

    }

    //  handle change to keycode

    //   Two handle mode

    if (powerState != lastPowerState) {

      if (powerState == 0) {

        // Neutral
        Keyboard.press('s');
        delay(KEYPRESS);
        Keyboard.releaseAll();

      } else {
        if (powerState > lastPowerState) {
          powerStep = powerState - lastPowerState;
          for (int i = 0 ; i < powerStep ; i++) {
            Keyboard.press('z');
            delay(KEYPRESS);
            Keyboard.releaseAll();
          }

        } else {
          powerStep = lastPowerState - powerState;
          for (int i = 0 ; i < powerStep ; i++) {
            Keyboard.press('q');
            delay(KEYPRESS);
            Keyboard.releaseAll();
          }
        }

      }

    }

    if (breakState != lastBreakState) {
      if (breakState == 0) {
        // Neutral
        Keyboard.press('m');
        delay(KEYPRESS);
        Keyboard.releaseAll();
      } else if (breakState == 9) {
        // EB
        Keyboard.press('/');
        delay(KEYPRESS);
        Keyboard.releaseAll();
      } else {
        if (breakState > lastBreakState) {
          breakStep = breakState - lastBreakState;
          for (int i = 0 ; i < breakStep ; i++) {
            Keyboard.press('.');
            delay(KEYPRESS);
            Keyboard.releaseAll();
          }

        } else {
          breakStep = lastBreakState - breakState;
          for (int i = 0 ; i < breakStep ; i++) {
            Keyboard.press(',');
            delay(KEYPRESS);
            Keyboard.releaseAll();
          }
        }
      }
    }

    lastPowerState = powerState;
    lastBreakState = breakState;

  }

}


void dumpAnalog (const char *str, const byte x, const byte y) {
  Serial.print (str);
  Serial.print (F(" analog: x = "));
  Serial.print (x);
  Serial.print (F(", y = "));
  Serial.println (y);
}



const char ctrlTypeUnknown[] PROGMEM = "Unknown";
const char ctrlTypeDualShock[] PROGMEM = "Dual Shock";
const char ctrlTypeDsWireless[] PROGMEM = "Dual Shock Wireless";
const char ctrlTypeGuitHero[] PROGMEM = "Guitar Hero";
const char ctrlTypeOutOfBounds[] PROGMEM = "(Out of bounds)";

const char* const controllerTypeStrings[PSCTRL_MAX + 1] PROGMEM = {
  ctrlTypeUnknown,
  ctrlTypeDualShock,
  ctrlTypeDsWireless,
  ctrlTypeGuitHero,
  ctrlTypeOutOfBounds
};


PsxControllerBitBang<PIN_PS2_ATT, PIN_PS2_CMD, PIN_PS2_DAT, PIN_PS2_CLK> psx;

boolean haveController = false;

void setup () {

  delay (300);

  Display.displaySet(2);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

  Keyboard.begin();

  Serial.begin (115200);
  Serial.println (F("Ready!"));
}

void loop () {
  static byte slx, sly, srx, sry;

  if (!haveController) {
    if (psx.begin ()) {
      Serial.println (F("Controller found!"));
      delay (300);
      if (!psx.enterConfigMode ()) {
        Serial.println (F("Cannot enter config mode"));
      } else {
        PsxControllerType ctype = psx.getControllerType ();
        PGM_BYTES_P cname = reinterpret_cast<PGM_BYTES_P> (pgm_read_ptr (&(controllerTypeStrings[ctype < PSCTRL_MAX ? static_cast<byte> (ctype) : PSCTRL_MAX])));
        Serial.print (F("Controller Type is: "));
        Serial.println (PSTR_TO_F (cname));

        if (!psx.enableAnalogSticks ()) {
          Serial.println (F("Cannot enable analog sticks"));
        }

        //~ if (!psx.setAnalogMode (false)) {
        //~ Serial.println (F("Cannot disable analog mode"));
        //~ }
        //~ delay (10);

        if (!psx.enableAnalogButtons ()) {
          Serial.println (F("Cannot enable analog buttons"));
        }

        if (!psx.exitConfigMode ()) {
          Serial.println (F("Cannot exit config mode"));
        }
      }

      haveController = true;
    }
  } else {
    if (!psx.read ()) {
      Serial.println (F("Controller lost :("));
      haveController = false;
    } else {
      dengo (psx.getButtonWord ());

      /*
      			byte lx, ly;
      			psx.getLeftAnalog (lx, ly);
      			if (lx != slx || ly != sly) {
      				dumpAnalog ("Left", lx, ly);
      				slx = lx;
      				sly = ly;
      			}

      			byte rx, ry;
      			psx.getRightAnalog (rx, ry);
      			if (rx != srx || ry != sry) {
      				dumpAnalog ("Right", rx, ry);
      				srx = rx;
      				sry = ry;
      			}

      */

    }
  }

  delay (1000 / 60);
}
