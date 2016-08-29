/* EEROM.h */
#include <inttypes.h>

class EEPROMClass
{
  public:
  uint8_t read(int) { };
  void write(int, uint8_t) { };
};

EEPROMClass EEPROM;

