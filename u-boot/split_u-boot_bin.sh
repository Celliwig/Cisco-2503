#!/bin/bash

ORIGINAL_BIN="u-boot-dtb.bin"

srec_cat ${ORIGINAL_BIN} -Binary -split 2 0 -o ${ORIGINAL_BIN}.even -Binary
srec_cat ${ORIGINAL_BIN} -Binary -split 2 1 -o ${ORIGINAL_BIN}.odd -Binary

python2.7 ../monitor/bin/flip_bits.py ${ORIGINAL_BIN}.odd ${ORIGINAL_BIN}.fw1
python2.7 ../monitor/bin/flip_bits.py ${ORIGINAL_BIN}.even ${ORIGINAL_BIN}.fw2

