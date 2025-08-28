#!/bin/python

rom=bytearray([0xEA]*65536)
rom[0x8000]=0xA9  # STA 
rom[0x8001]=0x54
rom[0x8002]=0x8D  # LDA
rom[0x8003]=0x32
rom[0x8004]=0x50
rom[0xFFFC]=0x4C  # JMP
rom[0xFFFD]=0x00
rom[0xFFFE]=0x80

with open("registerTest.bin", "wb") as out_file:
    out_file.write(rom)


###     Stores the value 0x54 in the A register then loads the value from the A register and writes it to adds 0x5032
