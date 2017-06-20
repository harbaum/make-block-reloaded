# Make:Block reloaded

This repository contains the source code for the updated Make:Block
project as published in [Make 04/2016](http://www.heise.de/make/inhalt/2016/4/52/).

The `orig` subdirectory contains a Linux environment to run the original
sketch publsihed in [Make 01/2015](http://www.heise.de/make/inhalt/2015/1/12/)
directly on a Linux PC. This setup simplifies development and is the 
foundation of a totally rewritten game engine.

![Splash screen](title.png) ![Game screen](game.png)

## Compilation

Download the whole repository to your local PC. Make sure the local
directory is named ```tetris```. Unless you intend to modify the
splash screen you should remove/rename the file ```bin2c.c``` as
it will be picked up by the Arduino IDE although it's not part of
the sketch itself.

Open the file ```tetris.ino``` in the Arduino IDE. Then install
the latest FastLED library e.g. from within the IDE's menu under
"sketch->install libraries". The FastLED lib is found in the "display"
section.

Now simply hit the build button. 