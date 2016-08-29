# Makefile

APP=tetris

$(APP): linux/main.cpp linux/FastLED.h $(APP).ino logo.ino text.ino title.ino tetris.h keys.ino score.ino audio.ino song.ino config.ino initials.ino linux/EEPROM.h
	g++ -o $@ linux/main.cpp -fno-exceptions -I. -Ilinux `sdl-config --cflags --libs`

bin2c: bin2c.c

logo.ino: logo.rgb bin2c
	echo "const unsigned char logo[] PROGMEM = {" > logo.ino
	./bin2c logo.rgb >> logo.ino
	echo "};" >> logo.ino

logo.rgb: logo.png Makefile
	convert logo.png -rotate -90 -flip logo.rgb

clean:
	rm -f $(APP) *~ */*~  

arduino:
	cp *.ino tetris.h ~/sketchbook/tetris
