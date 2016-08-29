/* EEROM.h */
#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>

class EEPROMClass
{
  public:

  uint8_t read(int a) { 
    uint8_t retval = 0;
    FILE *f = fopen("eeprom.bin", "rb");
    if(f) {
      if(fseek(f, a, SEEK_SET) >= 0)
	fread(&retval, 1l, 1l, f);
      fclose(f);
    }
    return retval;
  };

  void write(int a, uint8_t b) { 
    FILE *f = fopen("eeprom.bin", "r+b");
    if(!f) f = fopen("eeprom.bin", "wb");

    if(f) {
      if(fseek(f, a, SEEK_SET) >= 0) 
	fwrite(&b, 1l, 1l, f);
      fclose(f);
    } else
      printf("unable to open file for writing\n");
  };

  //Functionality to 'get' and 'put' objects to and from EEPROM.
  template< typename T > T &get( int idx, T &t ){
    //    printf("eeprom GET %ld @ %d\n", sizeof(T), idx);
    uint8_t *ptr = (uint8_t*) &t;
    for( int count = sizeof(T) ; count ; --count)  *ptr++=read(idx++);
    return t;
  }
    
  template< typename T > const T &put( int idx, const T &t ){
    //    printf("eeprom PUT %ld @ %d\n", sizeof(T), idx);
    const uint8_t *ptr = (const uint8_t*) &t;
    for( int count = sizeof(T) ; count ; --count)  write(idx++, *ptr++);
    return t;
    }
};

EEPROMClass EEPROM;

#endif // EEPROM_H
