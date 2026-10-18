#!/bin/sh
echo '=== Assignment 01: Task 2 Misc Counter Test ==='

echo '[1] Loading counter_driver.ko...'
insmod /lib/modules/5.15.0/counter_driver.ko
if [ $? -ne 0 ]; then
    echo 'ERROR: insmod failed!'
    exit 1
fi

echo '[2] Reading initial counter (auto-increment test)...'
echo -n 'Read 1: '
cat /dev/counter
echo -n 'Read 2: '
cat /dev/counter

echo '[3] Setting counter to 100...'
echo '100' > /dev/counter
echo -n 'Read back after setting 100: '
cat /dev/counter

echo '[4] Resetting counter to 0...'
echo '0' > /dev/counter
echo -n 'Read back after reset: '
cat /dev/counter

echo '[5] Checking /proc/counter_info...'
cat /proc/counter_info

echo '[6] Checking kernel logs (dmesg)...'
dmesg | grep counter_driver | tail -5

echo '=== Task 2 Test Finished ==='
