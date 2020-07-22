# Makefile

APP=makeblock

$(APP): linux/main.cpp linux/FastLED.h logo.ino text.ino mario.ino mario_lvl.ino title.ino makeblock.h keys.ino score.ino audio.ino song.ino config.ino initials.ino make-block-reloaded.ino linux/EEPROM.h
	g++ -o $@ linux/main.cpp -fno-exceptions -I. -Ilinux `sdl-config --cflags --libs`

tools/bin2c: tools/bin2c.c

tools/png2level: tools/png2level.c

logo.ino: logo.rgb logo_m.rgb tools/bin2c
	echo "const unsigned char logo[] PROGMEM = {" > logo.ino
	./tools/bin2c logo.rgb >> logo.ino
	echo "};" >> logo.ino
	echo "const unsigned char logo_m[] PROGMEM = {" >> logo.ino
	./tools/bin2c logo_m.rgb >> logo.ino
	echo "};" >> logo.ino

mario_lvl.ino: mario-1-1.rgb tools/png2level
	./tools/png2level mario-1-1.rgb > $@

#mario-%.png: mario-%.gif Makefile
#	convert $< -crop x208+0+8 +repage -adaptive-resize x13 $@

%.rgb: %.png Makefile
	convert $< -rotate -90 -flip $@

clean:
	rm -f $(APP) *~ */*~  

arduino:
	cp *.ino tetris.h ~/sketchbook/tetris
