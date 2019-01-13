default: test1a test1b

test1a:
	../hex2bin -r 256 0x300000 test1.hex test1a.bin
	cmp test1a.bin test1a.bin.expected

test1b:
	../hex2bin -r 32768 0 test1.hex test1b.bin
	cmp test1b.bin test1b.bin.expected
