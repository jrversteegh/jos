// JOS configuration
// Define these values in your sketch before including 
// JGLCD.h and define __JGLCD_CONFIG_H__ to override these
// values.

#ifndef __JGLCD_CONFIG_H__
#define __JGLCD_CONFIG_H__

namespace JOS {
  static const int display_w = 128;
  static const int display_h = 64;

  static const int h_px_per_chip = 64;
  static const int v_px_per_chip = 64;

  // Check datasheet for number of chip select pins
  static const int chip_select_pins = 2;

  // Chip timings
  static const int t_ddr = 320; // Data delay time
  static const int t_as = 140;  // Address setup time
  static const int t_dsw = 200; // Data setup time
  static const int t_wh = 450;  // E hi pulse width
  static const int t_wl = 450;  // E lo pulse width

  // Pin config
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  static const int p_csel[] = {33, 34, 31, 32};
  static const int p_rw = 35;
  static const int p_di = 36;
  static const int p_en = 37;
  static const int p_data[] = {22, 23, 24, 25, 26, 27, 28, 29};
#else
  static const int p_csel[] = {14, 15, 3, 2};
  static const int p_rw = 16;
  static const int p_di = 17;
  static const int p_en = 18;
  static const int p_data[] = {8, 9, 10, 11, 4, 5, 6, 7};
#endif

}  // Namespace JOS

#endif

