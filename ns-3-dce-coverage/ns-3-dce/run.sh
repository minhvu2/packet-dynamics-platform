#!/bin/bash

out=1
SCRIPT=$1
INTERVAL=$2
S2E_DIR="/home/minh/s2edir/s2e"


# Extract delay values from the S2E output files
#rm -f tmp.txt
#rm -f delay.txt
#rm -f sym-delay.txt
#cat ${S2E_DIR}/s2e-last/debug.txt | grep -oP 'Packetuid \K[0-9]*  Delay [0-9]*' > tmp.txt
#sed 's/Delay//g' tmp.txt > delay.txt
#cat ${S2E_DIR}/s2e-last/debug.txt | grep -oP '\(int64_t\) \K[0-9]*' > sym-delay.txt

cd delay/
rm -f delay[0-9]*.txt
rm -f symdelay*.txt
lcov --directory /home/minh/net-next-nuse --zerocounters

# Each set of delay values is put in a seperated file
while read -r line; do
  if [ "$line" == "123456789   123456789" ]; then
    rm -f "delay${out}.txt"
    mv tmp.txt "delay${out}.txt"
    (( out++ ))
  else
    echo "$line" >> tmp.txt
  fi 
done < delay.txt

out=1
while read -r line; do
  rm -f "symdelay${out}.txt"
  if [ $line -lt $INTERVAL ]; then 
    echo "$line" >> "symdelay${out}.txt"
    (( out++ ))
  fi
done < sym-delay.txt

cd ..

# Run the script and collect coverages
for (( i=1; i<out; i++ ))
do
  echo "$i"
  rm -rf file-*
  ./waf --run "${SCRIPT} -num=123456789 -delayFile=delay/delay${i}.txt -symDelayFile=delay/symdelay${i}.txt -maxDelay=${INTERVAL} -verbose=true"
  ./utils/lcov.sh /home/minh/net-next-nuse
  mv dce-run.info out/dce-run-$i.info
  CMD_OPT="$CMD_OPT"" -a out/dce-run-$i.info"
done

./utils/lcov/lcov --rc lcov_branch_coverage=1 -q ${CMD_OPT} -o out/dce-run.info
rm -f out/dce-run-*
