#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

static void Abort (char *fmt,...)
{
  va_list args;
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  exit (1);
}

unsigned long get_pixel(unsigned char *data, int x, int y) {
  unsigned char r = data[3*13*x+3*(12-y)+0];
  unsigned char g = data[3*13*x+3*(12-y)+1];
  unsigned char b = data[3*13*x+3*(12-y)+2];

  return 256*256*r + 256*g + b;
}

int main (int argc, char **argv)
{
  FILE  *inFile;
  time_t now     = time (NULL);

  if (argc != 2)
     Abort ("Usage: %s bin-file [> result]", argv[0]);

  if ((inFile = fopen(argv[1],"rb")) == NULL)
     Abort ("Cannot open %s\n", argv[1]);

  printf("/* level data for file %s at %.24s */\n",
	 argv[1], ctime(&now));

  // check for total colors
  fseek(inFile, 0, SEEK_END);
  int fsize = ftell(inFile);
  fseek(inFile, 0, SEEK_SET);

  if(fsize % (3*13))
    Abort ("File height %d not matching\n", fsize % (3*13));
  if(fsize / (3*13) > 255)
    Abort ("File width %d not matching\n", fsize % (3*13));

  int width = fsize / (3*13);

  char *img = malloc(fsize);
  fread(img, fsize, 1, inFile);
  fclose(inFile);
  
  // topleft corner sure is "air", bottom left is ground
  unsigned long c_air = get_pixel(img, 0, 12);
  unsigned long c_ground = get_pixel(img, 0, 0);

  char level[] = "level_1_1";

  printf("#include \"makeblock.h\"\n\n");
  
  // check for air color on ground level (gaps)
  printf("const uint8_t gaps_%s[] PROGMEM = {\n", level);
  for(int x=0;x<width;x++) {
    if(get_pixel(img, x, 0) != c_ground) {
      if(get_pixel(img, x, 0) == c_air) {
	printf("%d, ", x);
      } else
	Abort("no ground/air at %d -> %lx\n", x, get_pixel(img, x, 0));
    }
  }
  printf("0 };\n\n");

  // check for clouds
  printf("const uint8_t clouds_%s[][3] PROGMEM = {\n", level);
  for(int y=1;y<13;y++) {
    for(int x=0;x<width;x++) {
      if(get_pixel(img, x, y) == 0xffffff) {
	printf("{%d,", x);
	int len = 1;
	while(get_pixel(img, x+1, y) == 0xffffff) {
	  x++;
	  len++;
	}
	printf("%d,%d},", y, len);
      }
    }
  }
  printf("{0,0,0} };\n\n");
  
  // check for background hills/bushes
  printf("const uint8_t hills_%s[][2] PROGMEM = {\n", level);
  for(int x=0;x<width;x++) {
    // dark hill only counts the double height parts
    if((get_pixel(img, x, 1) == 0x008000) &&
       (get_pixel(img, x, 2) == 0x008000)) {
      printf("{%d,", x);
      int len = 1;
      while(get_pixel(img, x+1, 2) == 0x008000) {
	x++;
	len++;
      }
      printf("%d},", 0x80 + len);
    }
    // light hill is always only one unit high
    if(get_pixel(img, x, 1) == 0x80ff80) {
      printf("{%d,", x);
      int len = 1;
      while(get_pixel(img, x+1, 1) == 0x80ff80) {
	x++;
	len++;
      }
      printf("%d},", len);
    }
  }
  printf("{0,0} };\n\n");
  
  // check for (brick) walls
  printf("const uint8_t walls_%s[][3] PROGMEM = {\n", level);
  for(int y=1;y<13;y++) {
    for(int x=0;x<width;x++) {
      if(get_pixel(img, x, y) == 0x642606) {
	printf("{%d,", x);
	int len = 1;
	while((get_pixel(img, x+1, y) == 0x642606) ||
	      (get_pixel(img, x+1, y) == 0xff8000) ||  // qmarks
	      (get_pixel(img, x+1, y) == 0xff0000)) {  // and mushrooms are also "walls"
	  x++;
	  len++;
	}
	printf("%d,%d},", y, len);
      }
    }
  }
  printf("{0,0,0} };\n\n");
  
  // check for questionmarks and mushrooms
  printf("const uint8_t qmarks_%s[][2] PROGMEM = {\n", level);
  for(int y=1;y<13;y++) {
    for(int x=0;x<width;x++) {
      if(get_pixel(img, x, y) == 0xff8000)  // ordinary qmark
	printf("{%d,%d},", x, y);
      if(get_pixel(img, x, y) == 0xff0000)  // mushroom
	printf("{%d,%d},", x, y | 0x80);
    }
  }      
  printf("{0,0} };\n\n");

  // check for enemies
  printf("const uint8_t enemies_%s[][4] PROGMEM = {\n", level);
  for(int y=1;y<13;y++) {
    for(int x=0;x<width;x++) {
      if((get_pixel(img, x, y) == 0x800080) ||  // goomba
	 (get_pixel(img, x, y) == 0xF79837)) {  // turtle
	printf("{%d,%d,", x, y | ((get_pixel(img,x,y) == 0xF79837)?0x80:0x00));

	// get ground movement limits
	// search to the left for solid items on ground level
	int min_x = x;
	while((min_x > -1) &&
	      (get_pixel(img, min_x, 1) != 0x642606) &&  // walls
	      (get_pixel(img, min_x, 1) != 0xff8000) &&  // qmarks
	      (get_pixel(img, min_x, 1) != 0xff0000) &&  // mushrooms
	      (get_pixel(img, min_x, 1) != 0xFCBCB0) &&  // stairs
	      (get_pixel(img, min_x, 1) != 0x00ff00))    // pipes
	  min_x--;
	
	// search to the right for solid items on ground level
	int max_x = x;
	while((max_x <= width) &&
	      (get_pixel(img, max_x, 1) != 0x642606) &&  // walls
	      (get_pixel(img, max_x, 1) != 0xff8000) &&  // qmarks
	      (get_pixel(img, max_x, 1) != 0xff0000) &&  // mushrooms
	      (get_pixel(img, max_x, 1) != 0xFCBCB0) &&  // stairs
	      (get_pixel(img, max_x, 1) != 0x00ff00))    // pipes
	  max_x++;
	  
	printf("%d,%d},", min_x+1, max_x-1);
      }
    }
  }      
  printf("{0,0,0,0} };\n\n");

  // check for "stairs"
  printf("const uint8_t stairs_%s[][2] PROGMEM = {\n", level);
  for(int x=0;x<width;x++) {
    if(get_pixel(img, x, 1) == 0xFCBCB0) {
      printf("{%d,", x);
      int len = 1;
      int height = 1, lh;
      while(get_pixel(img, x, 1+height) == 0xFCBCB0) height++;
      lh = height;
      
      while(get_pixel(img, x+1, 1) == 0xFCBCB0) {
	while(get_pixel(img, x+1, 1+height) == 0xFCBCB0) height++;	
	x++;
	len++;
      }

      // len (width) of stair = bits 0..4
      // extra columns = bits 5..6
      // ascending flag = bit 7
      len = (len&0x1f) + ((len-height)<<5);
      if(lh == 1) len |= 0x80;
      
      printf("%d},", len);
    }
  }
  printf("{0,0} };\n\n");

  // check for "pipes"
  printf("const uint8_t pipes_%s[][2] PROGMEM = {\n", level);
  for(int x=0;x<width;x++) {
    if(get_pixel(img, x, 1) == 0x00ff00) {
      printf("{%d,", x);
      int height = 1;
      while(get_pixel(img, x, 1+height) == 0x00ff00) height++;
      while(get_pixel(img, ++x, 1) == 0x00ff00);
      printf("%d},", height);
    }
  }
  printf("{0,0} };\n\n");

  // search for flag and castle
  int flag = 0, castle = 0;
  for(int x=0;x<width;x++) {
    if(get_pixel(img, x, 2) == 0xBAFFBA)
      flag = x;
    if((get_pixel(img, x, 1) == 0x710705) && !castle)
      castle = x;
  }
  
  // height is always 13
  printf("const struct mario_levelS %s PROGMEM = {\n", level);
  printf("  .width = %d,\n", width);
  printf("  .flag = %d,\n", flag);
  printf("  .castle = %d,\n", castle);
  printf("  .gaps = gaps_%s,\n", level);
  printf("  .clouds = clouds_%s,\n", level);
  printf("  .hills = hills_%s,\n", level);
  printf("  .walls = walls_%s,\n", level);
  printf("  .qmarks = qmarks_%s,\n", level);
  printf("  .stairs = stairs_%s,\n", level);
  printf("  .pipes = pipes_%s,\n", level);
  printf("  .enemies = enemies_%s\n", level);
  printf("};\n");

  return (0);
}
