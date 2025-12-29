#!/bin/bash

# Disable THP for this session if possible, or run with 'numactl'
# echo never | sudo tee /sys/kernel/mm/transparent_hugepage/enabled

OUTPUT="tlb_results.csv"
echo "Pages,Time" >$OUTPUT

# We need to go much higher than 512 to see L2 TLB and potentially escape
# Huge Pages
# 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384

for i in 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384; do
	# For large page counts, use fewer trials
	TRIALS=1000000
	if [ $i -gt 512 ]; then TRIALS=100000; fi
	if [ $i -gt 4096 ]; then TRIALS=10000; fi
	# Use taskset to ensure we stay on one core
	RES=$(taskset -c 0 ./tlb $i $TRIALS | awk '{print $NF}')
	echo "$i,$RES" >>$OUTPUT
	echo "Tested $i pages: $RES ns"
done
# This should give clean, positive numbers and a very visible
# "staircase" representing L1 TLB, L2 TLB, and Page Table walks.
