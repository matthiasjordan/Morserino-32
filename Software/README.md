# Morselino-32
This is Morselino-32, a fork of Morserino-32 originally by Willi Kraml OE1WKL.

The aim of this fork is to provide a code base that is easier to maintain and extend.


# Morserino-32 
Morserino-32 multi-functional Morse code machine, based on ESP32

The "**source**" directory contains the program sources (you need an Arduino IDE with the necessary hardware definitions for the heltec module if you want to compile the source).

The "**binary**" directory contains the compiled code in binary format, ready for upload to the Morserino-32 via WiFi (see User Manual for instructions).


# How to build 

## General preparation

* Install Arduino IDE following instructions on https://www.arduino.cc/en/Guide/Linux
* Libraries besorgen (wie in morse_3_v1.3.ino, Zeilen 33ff. beschrieben)
** NICHT: Heltec-Libraries installieren wie in Installation Instructions beschrieben (Bash-Skript)
** Heltec-Libraries installieren wie beschrieben in https://docs.heltec.cn/#/en/user_manual/how_to_install_esp32_Arduino (via "Additional Board Managers")
*** In Linux, it might be necessary to install the Heltec ESP32 Dev-Boards library using the library manager. Then create symlinks from the Heltec subdirectories to the library directory. In the directory where the Heltec library is installed, do
**** ln -s Heltec_ESP32_Dev-Boards/src/oled/ .
**** ln -s Heltec_ESP32_Dev-Boards/src/lora/ .
** ClickButton als RAR runterladen, extrahieren, installieren
* If the compiler complains about Vext being declared a second time, check that the two declarations yield the same value and then comment out the one in the Morserino source code.


## Arduino IDE

* Start Arduino IDE, Voreinstellungen öffnen


## Eclipse Sloeber

* Choose "Arduino -> Import Project into Workspace." from the context menu
** Source Folder Location is <gitrepo>/Software/src/morse
** Build project

