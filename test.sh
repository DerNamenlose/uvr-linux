#!/bin/bash

# Test script showing how to use the values from dlogg-reader when being
#  called via -s

echo "INPUTS: $UVR_INPUTS"
for i in `seq 1 $UVR_INPUTS`; do
	INPUT_TYPE="UVR_INPUT_${i}_TYPE"
	INPUT_VALUE="UVR_INPUT_${i}_VALUE"
	echo "INPUT $i: type=${!INPUT_TYPE}, value=${!INPUT_VALUE}";
done

echo "OUTPUTS: $UVR_OUTPUTS"
for i in `seq 1 $UVR_OUTPUTS`; do
	OUTPUT_VALUE="UVR_OUTPUT_${i}_VALUE"
	echo "OUTPUT $i: value=${!OUTPUT_VALUE}";
done

echo "HEATREG: $UVR_HEATREGS"
for i in `seq 1 $UVR_HEATREGS`; do
	REG_CURRENT="UVR_HEATREG_${i}_VALUE_CURRENT"
	REG_TOTAL="UVR_HEATREG_${i}_VALUE_TOTAL"
	echo "HEATREG $i: current=${!REG_CURRENT}, value=${!REG_TOTAL}";
done
