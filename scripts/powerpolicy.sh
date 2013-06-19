#!/usr/bin/env bash
# take from http://hbfs.wordpress.com/2013/06/18/fast-path-finding-part-ii/
# invoke with performance or ondemand 
# enumerate found CPUs
cpus=$( grep processor /proc/cpuinfo | cut -d: -f 2 )
 
# set governor for each CPU
#
for cpu in ${cpus[@]}
do
 cpufreq-selector -c $cpu -g $1
done
