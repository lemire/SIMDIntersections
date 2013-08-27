#!/usr/bin/env bash
# taken from http://hbfs.wordpress.com/2013/06/18/fast-path-finding-part-ii/
# might require sudo apt-get install cpufrequtils
# invoke with performance or ondemand 
# type cpufreq-info or "cat /proc/cpuinfo" to check results
# enumerate found CPUs
cpus=$( grep processor /proc/cpuinfo | cut -d: -f 2 )
 
# set governor for each CPU
#
for cpu in ${cpus[@]}
do
  cpufreq-set -c $cpu -g $1
done
