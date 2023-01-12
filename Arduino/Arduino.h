/*
 * Arduino.h
 *
 *  Created on: Jan 12, 2023
 *      Author: larry
 */

#ifndef USER_ARDUINO_H_
#define USER_ARDUINO_H_

// GPIO pin states
enum {
	OUTPUT = 0,
	INPUT,
	INPUT_PULLUP,
	INPUT_PULLDOWN
};

#define PROGMEM
#define memcpy_P memcpy
#define pgm_read_byte(s) *(uint8_t *)s
#define pgm_read_word(s) *(uint16_t *)s

// wrapper methods
void delay(int i);
void pinMode(uint8_t u8Pin, int iMode);
uint8_t digitalRead(uint8_t u8Pin);
void digitalWrite(uint8_t u8Pin, uint8_t u8Value);

#endif /* USER_ARDUINO_H_ */
