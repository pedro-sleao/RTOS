avr-gcc -mmcu=atmega328p -Os main.c -o main.out
avr-objcopy -j .text -j .data -O ihex main.out main.hex
avrdude -P /dev/ttyUSB0 -c arduino -p atmega328p -b57600 -v -D -U flash:w:main.hex:i