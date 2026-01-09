#!/usr/bin/bash

# $ ./paging-multilevel-translate.py -s 5 -n 12 -c | head -n -1 | tail -n 48 > pyoutput.txt
# $ ./os.sh -s 5 -n 12 | tail -n 48 > myoutput.txt
# $ diff -u pyoutput.txt myoutput.txt

OFFSET_MASK=0x1F
OFFSET_SHIFT=5
VPN_MASK=0x7FE0
PD_MASK=0x3E0
PFNBASE_MASK=0x7F
VBIT_MASK=0x80
VBIT_SHIFT=7
PT_MASK=0x1F

output=$(./paging-multilevel-translate.py "$@")
# TRICK: echo command: To keep output newline format wrap it in double quotes

# Validate python script
header_lines=$(echo "$output" | grep '^ARG' | wc -l)
if ! [ ${header_lines} == "3" ]; then
	exit 1
fi

# Retrieve the whole physical memory
PA=$(echo "$output" | rg '^page\s+[[:digit:]]{1,3}: (.*)$' -N -o -r '$1')
PhysicalMemory=($PA)

# Retrieve the PDBR
PDBR=$(echo "$output" | rg '^PDBR: ([[:digit:]]{1,3}) .*$' -N -o -r '$1')

# Retrieve all the VAs to work with
VA=$(echo "$output" | rg '^Virtual Address ([[:xdigit:]]{1,4}): .*$' -N -o -r '$1')
i=0
for a in $VA; do
	VMA[i]=$((0x${a}))
	((++i))
done

GetMemoryTableEntry() {
	ENTRYSIZE=1
	TABLEBASE=$1
	INDEX=$2
	ENTRYADDR=$((((TABLEBASE << OFFSET_SHIFT)) | ((INDEX * ENTRYSIZE))))

	i=0
	for j in ${PhysicalMemory[@]}; do
		if [ $i == "$ENTRYADDR" ]; then
			# echo $j
			printf '%d' 0x$j
			return
		fi
		((++i))
	done
}

# @param PHYSICALADDRESS
GetMemoryData() {
	i=0
	for j in ${PhysicalMemory[@]}; do
		if [ $i == "$1" ]; then
			echo "$j"
			return
		fi
		((++i))
	done
}

for VIRTUALADDRESS in ${VMA[@]}; do
	printf 'Virtual Address 0x%04x:\n' $VIRTUALADDRESS

	VPO=$((VIRTUALADDRESS & OFFSET_MASK))
	VPN=$((((VIRTUALADDRESS & VPN_MASK)) >> OFFSET_SHIFT))

	# TLB Miss: PAGE WALK

	PDI=$((((VPN & PD_MASK)) >> 5))
	PDE=$(GetMemoryTableEntry $PDBR $PDI)
	V1=$((((PDE & VBIT_MASK)) >> VBIT_SHIFT))
	PTBA=$((PDE & PFNBASE_MASK))

	printf '  --> pde index:0x%x [decimal %d] pde contents:%#x (valid %d, pfn 0x%02x [decimal %d])\n' $PDI $PDI $PDE $V1 $PTBA $PTBA

	if [ "$V1" == "0" ]; then
		echo "      --> Fault (page directory entry not valid)"
		continue
	fi

	PTI=$((VPN & PT_MASK))
	PTE=$(GetMemoryTableEntry $PTBA $PTI)
	V2=$((((PTE & VBIT_MASK)) >> VBIT_SHIFT))
	PFN=$((PTE & PFNBASE_MASK))

	printf '    --> pte index:0x%x [decimal %d] pte contents:%#x (valid %d, pfn 0x%02x [decimal %d])\n' $PTI $PTI $PTE $V2 $PFN $PFN

	if [ "$V2" == "0" ]; then
		echo "      --> Fault (page table entry not valid)"
		continue
	fi

	# TLB_Insert(VPN, PTE.PFN, PTE.ProtectBits)
	# RetryInstruction()

	DATATYPE=1
	PFO=$VPO
	PHYSICALADDRESS=$((((PFN << OFFSET_SHIFT)) | ((PFO * DATATYPE))))
	DATA=$(GetMemoryData $PHYSICALADDRESS)

	printf '      --> Translates to Physical Address 0x%03x --> Value: 0x%s\n' $PHYSICALADDRESS $DATA
done
