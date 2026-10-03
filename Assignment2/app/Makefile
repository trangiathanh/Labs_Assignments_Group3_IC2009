CC = gcc
CFLAGS = -Wall -Wextra -pthread -O2

TARGET = sms_app
OBJS = sms_app.o circular_buffer.o sensor_thread.o logger_thread.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o *.log

