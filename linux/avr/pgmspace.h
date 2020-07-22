#define PSTR(a) a
void *pgm_read_ptr_near(const void *p) { return *(void**)p; };
uint32_t pgm_read_dword_near(const void *p) { return *(uint32_t*)p; };
uint8_t pgm_read_byte_near(const void *p) { return *(uint8_t*)p; };
#define strcpy_P(a,b) strcpy(a,b)
#define strcat_P(a,b) strcat(a,b)
#define PROGMEM
