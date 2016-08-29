
uint32_t pgm_read_dword_near(const void *p) { return *(uint32_t*)p; };
uint8_t pgm_read_byte(const void *p) { return *(uint8_t*)p; };
#define PROGMEM
