I used this sequence to break a single .hex output from Microchip's XC8 into pieces that can be burned onto a PIC 18F452 using the Minipro TL866CS:

```
    hex2bin -r 256 0x300000 ../io_pic_v2/io_pic.hex ../io_pic_v2/io_pic.config.bin
    hex2bin -r 32768 0 ../io_pic_v2/io_pic.hex ../io_pic_v2/io_pic.code.bin

    minipro -p PIC18F452 -c code -w io_pic.code.bin
    minipro -p PIC18F452 -e -c data -w io_pic.config.bin

    # optional verification step:
    minipro -p PIC18F452 -c code -r foo.bin
    cmp foo.bin io_pic.code.bin
    minipro -p PIC18F452 -c data -r foo.bin
    cmp foo.bin io_pic.config.bin
```

The function readhex() in readhex.c that hex2bin uses is very old code (thus C language) and only handles 3 record types (store bytes, set high address bytes, and end).  It probably could be improved.
