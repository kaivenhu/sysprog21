#!/bin/bash

option=${1}

MASSIF_OUT=massif-test.out
PERF_LIST="cache-misses,cache-references,instructions,cycles,
L1-dcache-load-misses,L1-dcache-loads,LLC-load-misses,
LLC-loads,LLC-store-misses,LLC-stores"

CPUID=0
ORIG_SCL=`cat /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor`
ORIG_NTURBO=`cat /sys/devices/system/cpu/intel_pstate/no_turbo`
ORIG_ASLR=`cat /proc/sys/kernel/randomize_va_space`

sudo sh -c "echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo"
sudo sh -c "echo performance > /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor"
sudo sh -c "echo 0 > /proc/sys/kernel/randomize_va_space"

if [ "${option}" == "locality" ]; then
    sudo valgrind --tool=massif --time-unit=B --massif-out-file=${MASSIF_OUT} \
        ./xs-unittest.exe locality
    sudo ms_print ${MASSIF_OUT}
    sync > /dev/null
    free > /dev/null
    sleep 1s
    echo ${PERF_LIST}
    sudo perf stat --repeat 10 -e "${PERF_LIST}" taskset 0x1 ./xs-unittest.exe locality
fi

if [ "${option}" == "benchmark" ]; then
    taskset 0x1 ./xs-unittest.exe benchmark > result.dat
    gnuplot script/plot.gp
fi


sudo sh -c "echo $ORIG_NTURBO > /sys/devices/system/cpu/intel_pstate/no_turbo"
sudo sh -c "echo $ORIG_SCL > /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor"
sudo sh -c "echo $ORIG_ASLR >  /proc/sys/kernel/randomize_va_space"
