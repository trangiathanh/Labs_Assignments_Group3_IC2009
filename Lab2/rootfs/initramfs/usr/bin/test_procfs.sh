#!/bin/sh

echo "========================================"
echo "=== LAB-02: procfs & sysfs Test ==="
echo "========================================"

MODULE="/lib/modules/5.15.0/lab2_driver.ko"
DEVICE="/dev/lab2"
SYSFS="/sys/class/lab2_class/lab2"
PROC="/proc/lab2_info"

# --------------------------------------------------
# [1] Load module
# --------------------------------------------------
echo
echo "[1] Checking lab2_driver module..."

if [ ! -f "$MODULE" ]; then
    echo "ERROR: Module not found: $MODULE"
    exit 1
fi

if lsmod | grep -q '^lab2_driver'; then
    echo "lab2_driver is already loaded."
else
    if ! insmod "$MODULE"; then
        echo "ERROR: insmod failed!"
        exit 1
    fi
    echo "PASS: lab2_driver loaded."
fi

# --------------------------------------------------
# [2] Check device node
# --------------------------------------------------
echo
echo "[2] Checking /dev/lab2..."

if [ ! -c "$DEVICE" ]; then
    mknod "$DEVICE" c 240 0 2>/dev/null

    if [ $? -ne 0 ]; then
        echo "ERROR: Cannot create $DEVICE"
        exit 1
    fi
fi

chmod 666 "$DEVICE" 2>/dev/null
ls -la "$DEVICE"

# --------------------------------------------------
# [3] Check procfs
# --------------------------------------------------
echo
echo "[3] Checking procfs..."

if [ ! -f "$PROC" ]; then
    echo "FAIL: $PROC does not exist!"
    exit 1
fi

echo "--- /proc/lab2_info (before write) ---"
cat "$PROC"

# --------------------------------------------------
# [4] Write new data
# --------------------------------------------------
echo
echo "[4] Writing test data..."

TEST_DATA="Embedded Linux Lab2 Test Data"

if ! printf '%s\n' "$TEST_DATA" > "$DEVICE"; then
    echo "FAIL: Cannot write to $DEVICE"
    exit 1
fi

echo "PASS: Write successful."

# --------------------------------------------------
# [5] Check procfs after write
# --------------------------------------------------
echo
echo "--- /proc/lab2_info (after write) ---"
cat "$PROC"

if cat "$PROC" | grep -q "Data length"; then
    echo "PASS: procfs information available."
else
    echo "FAIL: procfs information invalid!"
    exit 1
fi

if cat "$PROC" | grep -q "$TEST_DATA"; then
    echo "PASS: procfs contains last written data."
else
    echo "WARNING: Last data not found in procfs."
fi

# --------------------------------------------------
# [6] Check sysfs directory
# --------------------------------------------------
echo
echo "[6] Checking sysfs..."

if [ ! -d "$SYSFS" ]; then
    echo "FAIL: sysfs directory does not exist:"
    echo "$SYSFS"
    exit 1
fi

echo "sysfs directory:"
ls -la "$SYSFS"

# --------------------------------------------------
# [7] Check individual attributes
# --------------------------------------------------
echo
echo "[7] Checking sysfs attributes..."

for ATTR in buffer_len open_count last_data
do
    if [ ! -f "$SYSFS/$ATTR" ]; then
        echo "FAIL: Missing sysfs attribute: $ATTR"
        exit 1
    fi
done

echo "PASS: All sysfs attributes exist."

# --------------------------------------------------
# [8] Read sysfs attributes
# --------------------------------------------------
echo
echo "--- sysfs attributes ---"

BUFFER_LEN=$(cat "$SYSFS/buffer_len")
OPEN_COUNT=$(cat "$SYSFS/open_count")
LAST_DATA=$(cat "$SYSFS/last_data")

echo "buffer_len : $BUFFER_LEN"
echo "open_count : $OPEN_COUNT"
echo "last_data  : $LAST_DATA"

# --------------------------------------------------
# [9] Basic validation
# --------------------------------------------------
echo
echo "[9] Validating sysfs data..."

if [ "$BUFFER_LEN" -gt 0 ] 2>/dev/null; then
    echo "PASS: buffer_len > 0"
else
    echo "FAIL: buffer_len is invalid."
    exit 1
fi

if [ "$OPEN_COUNT" -gt 0 ] 2>/dev/null; then
    echo "PASS: open_count > 0"
else
    echo "WARNING: open_count is 0."
fi

# --------------------------------------------------
# [10] Kernel log
# --------------------------------------------------
echo
echo "[10] Kernel messages..."

dmesg | grep lab2 | tail -20

# --------------------------------------------------
# RESULT
# --------------------------------------------------
echo
echo "========================================"
echo "=== PROCFS/SYSFS TEST PASSED ==="
echo "========================================"

exit 0
