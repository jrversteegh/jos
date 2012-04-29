// JOS configuration
// Define these values in your sketch before including 
// JLCD.h and define __JLCD_CONFIG_H__ to override these
// values.

#ifndef __JLCD_CONFIG_H__
#define __JLCD_CONFIG_H__

namespace JOS {

// Characters per line
static const int line_chars = 16;
// Number of lines
static const int number_of_lines = 2;

// Arduino pins used for display
static const int pin_rs = 8;  // Register select pin
static const int pin_en = 9;  // Enable pin

// Data pins
static const int data_pins[] = {4, 5, 6, 7};  
static const int data_pin_count = sizeof(data_pins) / sizeof(data_pins[0]);

// Total screen characters
static const int char_count = number_of_lines * line_chars;

// Memory gap between lines
static const int line_gap = 64 - line_chars;
}


#endif
