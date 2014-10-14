coastermelt
===========

An effort to make open source firmware for burning anything other than Blu-Ray data onto plastic discs with a BD-R drive.

Still TOTALLY NOT REAL YET. Just a pie-in-the-sky reverse engineering effort. When details come along that I can publicize, they'll go here for now. Eventually this repo will become an open source firmware, I hope.

What it has
-----------

For the Samsung SE-506CB external Blu-Ray burner, it provides a way to install 'backdoored' firmware to support a set of programmatic and interactive reverse engineering tools.

Mac OS X only for now. Compiling the backdoor patch requires arm-none-eabi-gcc and friends, but it requires no specialized tools other than XCode if you use the included binary.

NOTE that there are NO copyrighted firmware images included here in this open source project! To be on the safe side, we don't include large disassemblies or reverse engineering databases either. This project includes tools written from scratch, notes based on guesswork and extensive experimentation. The installation process requires modifying an official (copyrighted) firmware image, which this project does not redistribute. The build system will automatically download this file from the official source.

~MeS`14
