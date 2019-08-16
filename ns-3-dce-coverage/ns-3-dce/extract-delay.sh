#!/bin/bash

S2E_DIR="/home/minh/s2edir/s2e"

rm -f tmp.txt
rm -f delay.txt
rm -f sym-delay.txt
rm -f delay/*.txt

cat ${S2E_DIR}/s2e-last/debug.txt | grep -oP 'Packetuid \K[0-9]*  Delay [0-9]*' > tmp.txt

sed 's/Delay//g' tmp.txt > delay.txt

cat ${S2E_DIR}/s2e-last/debug.txt | grep -oP '\(int64_t\) \K[0-9]*' > sym-delay.txt

mv delay.txt delay/

mv sym-delay.txt delay/
