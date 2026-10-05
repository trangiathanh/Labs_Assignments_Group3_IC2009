#!/bin/sh

echo "========================================"
echo "=== LAB-02: Device Driver Test ==="
echo "========================================"

MODULE="/lib/modules/5.15.0/lab2_driver.ko"
DEVICE="/dev/lab2"

# --------------------------------------------------
# [1] Kiểm tra kernel module
# --------------------------------------------------
echo
echo "[1] Checking module..."

if [ ! -f "$MODULE" ]; then
    echo "ERROR: Module not found: $MODULE"
    exit 1
fi

echo "Module found: $MODULE"

# Chỉ insmod nếu module chưa được load
if lsmod | grep -q '^lab2_driver'; then
    echo "lab2_driver is already loaded."
else
    echo "Loading lab2_driver..."
    if ! insmod "$MODULE"; then
        echo "ERROR: insmod failed!"
        exit 1
    fi
    echo "lab2_driver loaded successfully."
fi

# --------------------------------------------------
# [2] Kiểm tra lsmod
# --------------------------------------------------
echo
echo "[2] Checking lsmod..."

if lsmod | grep -q '^lab2_driver'; then
    echo "PASS: lab2_driver is loaded."
    lsmod | grep lab2
else
    echo "FAIL: lab2_driver is not loaded!"
    exit 1
fi

# --------------------------------------------------
# [3] Tạo /dev/lab2
# --------------------------------------------------
echo
echo "[3] Creating device node..."

if [ ! -c "$DEVICE" ]; then
    mknod "$DEVICE" c 240 0 2>/dev/null

    if [ $? -ne 0 ]; then
        echo "ERROR: Cannot create $DEVICE"
        exit 1
    fi
fi

chmod 666 "$DEVICE" 2>/dev/null

echo "Device:"
ls -la "$DEVICE"

# --------------------------------------------------
# [4] Test WRITE
# --------------------------------------------------
echo
echo "[4] Testing WRITE..."

TEST_DATA="Hello from userspace, LAB-02!"

if ! printf '%s\n' "$TEST_DATA" > "$DEVICE"; then
    echo "FAIL: Write to $DEVICE failed!"
    exit 1
fi

echo "PASS: Write successful."

# --------------------------------------------------
# [5] Test READ
# --------------------------------------------------
echo
echo "[5] Testing READ..."

DATA=$(cat "$DEVICE")

echo "Expected: [$TEST_DATA]"
echo "Received: [$DATA]"

if [ "$DATA" = "$TEST_DATA" ]; then
    echo "PASS: Read/write data matches."
else
    echo "FAIL: Read/write data does not match!"
    exit 1
fi

# --------------------------------------------------
# [6] Multiple WRITE/READ test
# --------------------------------------------------
echo
echo "[6] Multiple write/read test..."

for i in 1 2 3
do
    TEST_MSG="Message_$i"

    printf '%s\n' "$TEST_MSG" > "$DEVICE"
    READ_MSG=$(cat "$DEVICE")

    echo "Write : [$TEST_MSG]"
    echo "Read  : [$READ_MSG]"

    if [ "$READ_MSG" != "$TEST_MSG" ]; then
        echo "FAIL: Message_$i mismatch!"
        exit 1
    fi
done

echo "PASS: Multiple write/read test."

# --------------------------------------------------
# [7] Kernel log
# --------------------------------------------------
echo
echo "[7] Kernel messages..."

dmesg | grep lab2 | tail -20

# --------------------------------------------------
# RESULT
# --------------------------------------------------
echo
echo "========================================"
echo "=== CHARACTER DEVICE TEST PASSED ==="
echo "========================================"

exit 0
