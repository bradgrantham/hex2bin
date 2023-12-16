hex2bin extracts all the bytes in a HEX file for a given range of addresses.  Those addresses are output into a binary file, suitable for programming with the Minipro TL866CS or possibly using other tools.

hexinfo prints out all the contiguous regions in a HEX file, for verification or to better understand a tool's output.  hexinfo will also print out if there are checksum errors and if the HEX file specifies an address to start execution, which I believe is set by GNU binutils' objcopy for an ELF file.

I used this sequence to break a single .hex output from Microchip's XC8 into pieces that can be burned onto a PIC 18F452:

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

Using the -c option to program configuration fuses for PIC18F devices using recent versions of MINIPRO:

```
    proctype=PIC18F2620@DIP28
    source=example.hex

    ./hex2bin -r 65536 0x00000000 "$source" temp.bin
    ./hex2bin -r 256 0x00200000 "$source" temp.eeprom.bin
    ./hex2bin -c 8 14 0x00300000 "$source" temp.fuses.conf

    minipro -p $proctype --erase || exit
    minipro -p $proctype --page code --skip_erase --write temp.bin || exit
    minipro -p $proctype --page data --skip_erase --write temp.eeprom.bin || exit
    minipro -p $proctype --page config --skip_erase --write temp.fuses.conf || exit
    minipro -p $proctype --page config --skip_erase --verify temp.fuses.conf || exit
```

The function readhex() in readhex.c that hex2bin uses is very old code (thus C language).  It probably could be improved.

I don't actively use this tool any more.  I generally have been using STM32 parts, and the [stm32flash](https://github.com/bradgrantham/stm32flash) tool I use can read .hex files directly.  But if you submit a pull request, I'll try to merge within a week or so or start a thread about the merge.  Thank you!

Thanks to other individual contributors!
* [milesfrain](https://github.com/milesfrain)
* [bootladder](https://github.com/bootladder)
* [eegerferenc](https://github.com/eegerferenc/hex2bin) for addition of fused config output
