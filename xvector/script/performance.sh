#!/bin/bash

CPUID=0
ORIG_SCL=`cat /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor`
ORIG_NTURBO=`cat /sys/devices/system/cpu/intel_pstate/no_turbo`
ORIG_ASLR=`cat /proc/sys/kernel/randomize_va_space`

sudo sh -c "echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo"
sudo sh -c "echo performance > /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor"
sudo sh -c "echo 0 > /proc/sys/kernel/randomize_va_space"

taskset 0x1 ./xvec-unittest.exe benchmark > result.dat
gnuplot script/plot.gp


sudo sh -c "echo $ORIG_NTURBO > /sys/devices/system/cpu/intel_pstate/no_turbo"
sudo sh -c "echo $ORIG_SCL > /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor"
sudo sh -c "echo $ORIG_ASLR >  /proc/sys/kernel/randomize_va_space"
