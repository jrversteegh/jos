// JOS configuration
// Override these values in your sketch before including 
// JOS.h and define __JOS_CONFIG_H__ to override these
// values.

#ifndef __JOS_CONFIG_H__
#define __JOS_CONFIG_H__

// Panic LED pin
#ifdef __AVR_ATmega1280__
#define PANIC_LED_PIN 13
#else
#define PANIC_LED_PIN 1
#endif
// Reboot on panic. Requires a bootloader that can handle dogs!
#define PANIC_REBOOT 0

#endif
