# Labs & Assignments - Embedded Linux Systems

Tài liệu và mã nguồn thực hành cho khóa học **Hệ thống Linux Nhúng (Embedded Linux Systems)**, tập trung vào kiến trúc **ARMv7-A (Cortex-A9)** trên môi trường giả lập **QEMU vexpress-a9**.

## 📚 Danh sách các Bài Lab (Labs)

### 🔹 [LAB-01] Cấu hình Kernel & Khởi động Hệ thống Nhúng (ARMv7)
- **Mục tiêu:** Nắm vững toàn bộ chuỗi khởi động (*boot sequence*) của Linux nhúng từ ROM/Bootloader đến Userspace.
- **Nội dung thực hiện:**
  - Cross-compile **Linux Kernel 5.15 LTS** cho target ARMv7 Cortex-A9 với cấu hình `vexpress_defconfig`.
  - Biên dịch và sử dụng **U-Boot 2022.04** làm bootloader, cấu hình các biến môi trường (`bootargs`, `bootcmd`).
  - Xây dựng Root Filesystem tối giản bằng **BusyBox 1.35.0** (static linking) và đóng gói thành `initramfs.cpio.gz`.
  - Boot thành công hệ thống trên môi trường giả lập **QEMU `vexpress-a9`**.

### 🔹 [LAB-02] Viết Device Driver & Quản lý File System
- **Mục tiêu:** Hiểu vòng đời Kernel Module, tự viết Character Device Driver và quản lý MTD Flash Filesystem.
- **Nội dung thực hiện:**
  - Viết Character Device Driver (`lab2_driver.ko`) hỗ trợ `open`, `read`, `write`, `release` với cơ chế khóa `DEFINE_MUTEX`.
  - Trích xuất thông tin driver ra Userspace qua `/proc/lab2_info` (procfs) và `/sys/class/lab2_class/` (sysfs).
  - Sử dụng `nandsim` mô phỏng NAND Flash, phân vùng MTD, định dạng và kiểm tra tính bền vững (*persistence*) của **JFFS2**.
  - Tích hợp tự động nạp module (`insmod`) khi hệ thống boot thông qua script `/etc/init.d/rcS` của BusyBox init.

### 🔹 [LAB-03] Debugging Kernel & Phân tích Hiệu năng Ứng dụng Nhúng
- **Mục tiêu:** Thành thạo các công cụ remote debug, phát hiện memory leak, profiling CPU/Syscall và đo lường latency real-time.
- **Nội dung thực hiện:**
  - **Remote Debugging:** Kết nối `gdb-multiarch` (Host) với `gdbserver` (QEMU ARM) qua TCP port 1234 để phân tích call stack và khắc phục lỗi crash.
  - **Memory Analysis:** Dùng **Valgrind** bắt lỗi *memory leak* (Heap summary trace), sửa code và theo dõi `VmRSS`/`VmSize` qua `/proc/PID/status`.
  - **Profiling:** Sử dụng **strace** thống kê tần suất system call và **perf** (Flame Graph) tìm CPU hotspot.
  - **Real-Time Analysis:** Đo worst-case latency bằng **cyclictest**, đánh giá tác động của `PREEMPT_RT` kernel và vẽ biểu đồ Histogram với `gnuplot`.

---

## 📝 Danh sách các Bài tập lớn (Assignments)

### 🔸 [Assignment 01] How to Write a Device Driver
- **Task 1: IOCTL Interface Driver (40 điểm)** (`Assignment1/task1_ioctl/`)
  - Mở rộng Character Driver với 4 lệnh IOCTL: `ASGN1_RESET_BUFFER` (reset bộ đệm), `ASGN1_GET_STATS` (trả về struct thống kê), `ASGN1_SET_MODE` (chuyển đổi Echo/Kernel mode), `ASGN1_GET_VERSION` (chuỗi phiên bản).
  - Sử dụng khóa Mutex bảo vệ thao tác đọc/ghi và tránh tranh chấp dữ liệu.
- **Task 2: Misc Device Driver - Counter Device (30 điểm)** (`Assignment1/task2_misc/`)
  - Viết Misc Device Driver tạo node `/dev/counter` (`MISC_MAJOR = 10`).
  - Quản lý biến đếm bằng `atomic_t` đảm bảo thread-safety trong môi trường SMP.
  - Tự động tăng khi `read`, reset/set giá trị qua `write` và xuất thống kê ra `/proc/counter_info`.
- **Task 3 & 4: Báo cáo Phân tích Thiết kế & Lý thuyết (30 điểm)** (`Assignment1/bao_cao/`)
  - Vẽ sơ đồ kiến trúc luồng dữ liệu (VFS, Syscalls, IOCTL), so sánh 3 loại driver (Character, Block, Network), giải thích cơ chế `copy_to_user`, reference counting và xử lý `*ppos` trong hàm `read`.

---

### 🔸 [Assignment 02] Capstone Project - Sensor Monitoring System (SMS)
- **Task 1: Kiến trúc & Triển khai Ứng dụng Đa luồng (30 điểm)** (`Assignment2/app/` & `driver/`)
  - **Kernel Driver (`sms_sensor.ko`):** Mô phỏng cảm biến nhiệt độ/độ ẩm, xuất chuỗi CSV qua `/dev/sms_sensor`, thống kê qua `/proc/sms_stats` và điều khiển qua `ioctl`.
  - **Userspace App (`sms_app`):** Thiết kế 4 POSIX threads hoạt động song song:
    - `Sensor Thread`: Đọc cảm biến theo chu kỳ (SCHED_FIFO Real-Time) đẩy vào `shared_buffer`.
    - `Processor Thread`: Tính trung bình trượt (rolling average), kiểm tra ngưỡng và phát cảnh báo vào `alarm_queue`.
    - `Alert Thread`: Nhận thông báo qua POSIX Message Queue (`mq_receive`) và xuất cảnh báo.
    - `Logger Thread`: Nhận log message và ghi ra file `/tmp/sms.log` với cơ chế log rotation (>1MB).
- **Task 2: Quản lý Bộ nhớ & Tích hợp Driver (20 điểm)**
  - Kiểm tra memory leak bằng **Valgrind** (đảm bảo 0 bytes lost), phân tích memory footprint (`VmRSS`, `VmSize`) qua `/proc/PID/status`.
- **Task 3: Debug & Profiling Ứng dụng (20 điểm)**
  - Thực hiện phiên debug đa luồng bằng **GDB**, phân tích pattern system calls với **strace** và tìm CPU hotspot bằng **perf**.
- **Task 4: Phân tích Real-Time & Tối ưu hóa (15 điểm)**
  - Đo lường Jitter/Latency của Sensor Thread, tạo Histogram phân phối thời gian và so sánh hiệu quả giữa `SCHED_FIFO` vs `SCHED_OTHER`.
- **Task 5: Báo cáo Thiết kế & Video Demo (15 điểm)** (`Assignment2/bao_cao/` & `demo/`)
  - Sơ đồ kiến trúc hệ thống, State Machine, phân tích Synchronization / Resource Allocation Graph chứng minh không deadlock, cùng video demo 3-5 phút.

---

## 📁 Cấu trúc Repository

```text
LABS
├── LAB-01/                                # LAB-01: Cấu hình Kernel & Khởi động Hệ thống Nhúng (ARMv7)
│   ├── kernel/                            # Thư mục chứa nguồn và cấu hình Linux Kernel
│   │   ├── linux-5.15.tar.xz              # Mã nguồn nén Linux Kernel 5.15 LTS
│   │   └── linux-5.15/                    # Cây mã nguồn kernel sau khi giải nén
│   │       ├── .config                    # File cấu hình kernel sau khi nạp vexpress_defconfig & menuconfig
│   │       └── arch/arm/boot/             # Thư mục chứa kết quả build zImage và DTB
│   │           ├── zImage                 # Compressed Kernel Image dành cho kiến trúc ARMv7
│   │           └── dts/
│   │               └── vexpress-v2p-ca9.dtb # Device Tree Blob dành cho QEMU vexpress-a9
│   ├── uboot/                             # Thư mục chứa nguồn U-Boot Bootloader
│   │   ├── u-boot-2022.04.tar.bz2         # Mã nguồn nén U-Boot 2022.04
│   │   └── u-boot-2022.04/
│   │       ├── u-boot                     # File thực thi ELF U-Boot
│   │       └── u-boot.bin                 # U-Boot binary image
│   ├── rootfs/                            # Thư mục nguồn tạo Root Filesystem bằng BusyBox
│   │   ├── busybox-1.35.0.tar.bz2         # Mã nguồn nén BusyBox 1.35.0
│   │   ├── busybox-1.35.0/                # Mã nguồn BusyBox (biên dịch tĩnh / static link)
│   │   │   └── _install/                  # Bộ khung file thực thi sinh ra sau khi make install
│   │   └── initramfs/                     # Cấu trúc thư mục FHS hoàn chỉnh cho initramfs
│   │       ├── bin/                       # Các lệnh BusyBox (ls, sh, cat, cp...)
│   │       ├── sbin/                      # Các lệnh quản trị (init, reboot, mknod...)
│   │       ├── usr/                       # Thư mục chứa ứng dụng người dùng
│   │       │   ├── bin/
│   │       │   └── sbin/
│   │       ├── dev/                       # Các Device Nodes cơ bản
│   │       │   ├── console                # Character device console (major 5, minor 1)
│   │       │   ├── null                   # Character device null (major 1, minor 3)
│   │       │   ├── zero                   # Character device zero (major 1, minor 5)
│   │       │   └── random                 # Character device random (major 1, minor 8)
│   │       ├── proc/                      # Mount point cho procfs
│   │       ├── sys/                       # Mount point cho sysfs
│   │       ├── tmp/                       # Mount point cho tmpfs
│   │       ├── etc/                       # Các file cấu hình hệ thống
│   │       │   ├── inittab                # File cấu hình tiến trình BusyBox init
│   │       │   ├── passwd                 # File quản lý người dùng (root user)
│   │       │   ├── hostname               # File lưu tên hostname (embedded-lab)
│   │       │   └── init.d/
│   │       │       └── rcS                # Script khởi chạy đầu tiên khi boot (sysinit)
│   │       └── init                       # File thực thi /init khởi tạo mounting
│   └── output/                            # Các file nén và hình ảnh hoàn chỉnh của LAB-01
│       ├── zImage                         # Sao chép Linux Kernel Image
│       ├── vexpress-v2p-ca9.dtb           # Sao chép Device Tree Blob
│       └── initramfs.cpio.gz              # File đóng gói cpio archive nén gzip
│
├── LAB-02/                                # LAB-02: Viết Device Driver & Quản lý File System
│   ├── driver/                            # Mã nguồn Kernel Module Character Device
│   │   ├── lab2_driver.c                  # Driver hỗ trợ read/write/open/release, procfs, sysfs
│   │   ├── lab2_driver.ko                 # Kernel Module đã biên dịch chéo cho ARMv7
│   │   └── Makefile                       # KBuild Makefile để biên dịch kernel module
│   ├── rootfs/
│   │   └── initramfs/                     # RootFS mở rộng kế thừa từ LAB-01
│   │       ├── lib/
│   │       │   └── modules/
│   │       │       └── 5.15.0/
│   │       │           └── lab2_driver.ko # Kernel module tự động load khi boot qua rcS
│   │       ├── usr/
│   │       │   └── bin/
│   │       │       ├── test_driver.sh     # Script tự động kiểm tra read/write trên /dev/lab2
│   │       │       └── test_procfs.sh     # Script tự động kiểm tra /proc/lab2_info & sysfs
│   │       └── etc/
│   │           └── init.d/
│   │               └── rcS                # Tự động thực thi insmod & mknod /dev/lab2
│   ├── mtd/                               # Thực hành MTD Flash & JFFS2 Filesystem (trên Host)
│   │   ├── rootfs.jffs2                   # JFFS2 Image tạo ra từ mkfs.jffs2
│   │   └── dump.bin                       # Binary dump từ MTD0 bằng nanddump
│   └── output/
│       └── initramfs_lab2.cpio.gz         # RootFS hoàn chỉnh chứa driver và test script
│
├── LAB-03/                                # LAB-03: Debugging Kernel & Phân tích Hiệu năng Ứng dụng
│   ├── apps/                              # Mã nguồn các ứng dụng thử nghiệm
│   │   ├── buggy_app.c                    # Mã nguồn C chứa các lỗi cố ý (Segfault, Array Out-of-Bounds)
│   │   ├── buggy_app                      # File thực thi ARMv7 biên dịch kèm debug symbols (-g -O0)
│   │   ├── leaky_app.c                    # Mã nguồn C chứa lỗi memory leak trên Heap
│   │   ├── leaky_app                      # File thực thi x86_64 chạy thử trên Host
│   │   ├── leaky_app_fixed                # Mã nguồn đã sửa hoàn toàn lỗi giải phóng bộ nhớ
│   │   ├── perf_target.c                  # Ứng dụng tính toán nặng dùng để profiling
│   │   ├── perf_target                    # File thực thi x86_64 cho Host
│   │   └── perf_target_arm                # File thực thi ARMv7 cho QEMU
│   ├── rootfs/
│   │   └── initramfs/
│   │       └── usr/
│   │           └── bin/
│   │               ├── gdbserver          # Binary gdbserver dành cho kiến trúc ARM
│   │               ├── buggy_app          # Target debug từ gdb-multiarch qua TCP port 1234
│   │               └── perf_target_arm    # Target chạy strace/perf trên QEMU
│   └── output/
│       └── initramfs_lab3.cpio.gz         # RootFS tích hợp các công cụ debug
│
Labs_Assignments_Group3_IC2009/
├── LAB-01/               # Lab 1: Cấu hình Kernel, U-Boot Bootloader & BusyBox RootFS
├── LAB-02/               # Lab 2: Character Device Driver, procfs/sysfs, MTD & JFFS2
├── LAB-03/               # Lab 3: Remote Debugging (GDB), Valgrind, perf, strace & cyclictest
├── Assignment1/          # Assignment 1: How to Write a Device Driver
│   ├── task1_ioctl/      #   Task 1: Character Driver hỗ trợ IOCTL Interface & Mutex
│   ├── task2_misc/       #   Task 2: Misc Device Counter Driver dùng Atomic Operations
│   ├── logs/             #   Nhật ký kiểm thử (dmesg, test outputs)
│   └── bao_cao/          #   Báo cáo kỹ thuật PDF & Câu hỏi lý thuyết
└── Assignment2/          # Assignment 2: Capstone Project - Sensor Monitoring System (SMS)
    ├── driver/           #   Kernel Driver cảm biến (/dev/sms_sensor, /proc/sms_stats)
    ├── app/              #   Ứng dụng đa luồng POSIX (Sensor, Processor, Alert, Logger)
    ├── logs/             #   Kết quả phân tích Valgrind, strace, perf & Jitter report
    ├── demo/             #   Video Demo hoạt động hệ thống
    └── bao_cao/          #   Báo cáo thiết kế kiến trúc PDF
