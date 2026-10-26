#include <stdio.h>

void heavy_task_A() {
    // Vòng lặp giả lập tác vụ tốn CPU
    for(volatile int i = 0; i < 20000000; i++);
}

void heavy_task_B() {
    // Vòng lặp dài gấp đôi task A
    for(volatile int i = 0; i < 40000000; i++);
}

int main(void) {
    printf("=== Profiling App - LAB-03 Performance Analysis ===\n");
    for(int i = 0; i < 5; i++) {
        heavy_task_A();
    }
    heavy_task_B();
    printf("Tasks completed. Generating profiling data...\n");
    return 0;
}
