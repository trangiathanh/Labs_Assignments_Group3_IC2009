#!/bin/bash
echo "=== 1. Building Kernel Driver ==="
cd ../driver
make clean && make

echo "=== 2. Loading Kernel Driver ==="
sudo rmmod sms_sensor_driver 2>/dev/null
sudo insmod sms_sensor_driver.ko

echo "=== 3. Creating Device Node ==="
MAJOR=$(awk '$2=="sms_sensor" {print $1}' /proc/devices)
echo "Detected major number: $MAJOR"
sudo rm -f /dev/sms_sensor
sudo mknod /dev/sms_sensor c $MAJOR 0
sudo chmod 666 /dev/sms_sensor

echo "=== 4. Building Userspace App ==="
cd ../app
make clean && make

echo "=== 5. Running sms_app (Press Ctrl+C to stop) ==="
./sms_app

