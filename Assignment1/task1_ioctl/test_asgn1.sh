#!/bin/sh
echo '=== Assignment 01: Task 1 Driver Test ==='

echo '[1] Loading asgn1_driver.ko...'
insmod /lib/modules/5.15.0/asgn1_driver.ko
if [ $? -ne 0 ]; then
    echo 'ERROR: insmod failed!'
    exit 1
fi

echo '[2] Checking loaded module...'
lsmod | grep asgn1

echo '[3] Creating device node /dev/asgn1...'
mknod /dev/asgn1 c 242 0 2>/dev/null || true
chmod 666 /dev/asgn1

echo '[4] Testing WRITE in default Echo Mode...'
echo 'Hello from proGion' > /dev/asgn1

echo '[5] Testing READ...'
echo -n 'Read result: '
cat /dev/asgn1

echo '[6] Checking kernel logs (dmesg)...'
dmesg | grep asgn1_driver | tail -10

echo '=== Task 1 Test Finished ==='
